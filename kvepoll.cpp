
#include "kvepoll.h"
#include "socket.h"
#include "threadpool.h"
#include "kvconfig.h"
#include "kvhelper.h"
#include "simplekvsvr.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

using std::cout;
using std::endl;

namespace mmtraining {

KVEpoll::KVEpoll(ServerSocket *server, WorkQueue *queue, KVStore *kvStore) {
    epollfd = epoll_create(EPOLLSIZE);
    this->server = server;
    this->queue = queue;
    this->kvStore = kvStore;
    if (epollfd < 0) {
	cout << "KVEpoll::KVEpoll create epoll fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    } 

    buckets = new vector<KVEpollNode>[NEPOLLBUCKET];
    if (buckets == NULL) {
	cout << "KVEpoll::KVEpoll init buckets fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    }

    mutexs = new pthread_mutex_t[NEPOLLBUCKET];
    if (mutexs == NULL) {
	cout << "KVEpoll::KVEpoll init mutexs fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    }

    for (int i = 0; i < NEPOLLBUCKET; ++ i) {
	if (pthread_mutex_init(&mutexs[i], NULL) != 0) {
	    cout << "KVEpoll::KVEpoll mutex init fail" << endl;
	    cout << "Exit" << endl;
	    exit(1);
	}
    }

    events = new epoll_event[EVENTARRAYSIZE];
    if (events == NULL) {
	cout << "KVEpoll::KVEpoll init events fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    }

    serverfd = server->GetFD();
    //add server fd into epoll
    struct epoll_event event;
    event.data.fd = serverfd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event) != 0) {
	cout << "SimpleKVSvr::DaemonRun add server into epoll fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

KVEpoll::~KVEpoll() {
    for (int i = 0; i < NEPOLLBUCKET; ++ i) {
	pthread_mutex_destroy(&mutexs[i]);
	buckets[i].clear();
    }

    delete buckets;
    delete mutexs;
    delete events;
}

int KVEpoll::OneTimeLoop() {


    int nfd = epoll_wait(epollfd, events, EVENTARRAYSIZE, -1);
    if (nfd == -1) {
	cout << "KVEpoll::OneTimeLoop epoll_wait fail:" << strerror(errno) << endl;
	return -1;
    }

    for (int i = 0; i < nfd; ++ i) {

	if (events[i].data.fd == serverfd && (events[i].events & EPOLLIN)) { 
	    ServerFunc();
	} else if ((events[i].events & EPOLLIN) && events[i].data.fd) { 
	    ClientFunc(events[i]);
	}
	    
    }

    return 0;
}

int KVEpoll::ServerFunc() {
    int clientfd = server->Accept1();
    if (clientfd < 0) {
	cout << "KVEpoll::ServerFunc server accept client error" << endl;
	return -1;
    } else if (SetNonBlocking(clientfd) != 0) {
	cout << "KVEpoll::ServerFunc server set clientfd " << clientfd 
	     << " nonblocking error" << endl;
	close(clientfd);
	return -2;
    } 

    struct epoll_event event;
    event.data.fd = clientfd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event) != 0) {
	cout << "KVEpoll::ServerFunc server add clientfd into epoll error:"
	     << strerror(errno) << endl;
	close(clientfd);
	return -3;
    }

    AddClient(clientfd);
    //cout << "mainthread accept a socket" << endl;

    return 0;
}

int KVEpoll::ClientFunc(struct epoll_event event) {
    //cout << "a new requese comes" << endl;
    SimpleKVSvrWork *work = new SimpleKVSvrWork(event.data.fd, kvStore, this);
    if (work == NULL) {
	cout << "main thread can not create a work "
	     << "because not enough memory" << endl;
	return -1;
    } else if (queue->AddWork(work) == -1) {
	cout << "SimpleKVSvr::DaemonRun server can not add work" << endl;
	return -2;
    }

    return 0;
}


void KVEpoll::AddClient(int clientfd) {
    int index = HashFunc(clientfd);
    GetMutex(&mutexs[index]);
    int loc = FindInBucket(index, clientfd);
    if (loc == -1) {
	buckets[index].push_back(KVEpollNode(clientfd));
    } else {
	cout << "KVEpoll::AddClient fail, client exist" << endl;
    }
    ReleaseMutex(&mutexs[index]);
}

int KVEpoll::DeleteClient(int clientfd) {
    int ret = 0;

    int index = HashFunc(clientfd);
    GetMutex(&mutexs[index]);
    int loc = FindInBucket(index, clientfd);
    if (loc != -1) {
	//delete id in epoll
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, clientfd, NULL) != 0) {
	    cout << "KVEpoll::DeleteClient delete client fd " << clientfd << " in epoll "
		 << " fail" << strerror(errno) << endl;
	    ret = -1;
	}
	buckets[index].erase(buckets[index].begin() + loc);
    } else {
	ret = -1;
    }
    ReleaseMutex(&mutexs[index]);
    
    return ret;
}

int KVEpoll::UpdateClient(int clientfd) {
    int ret = 0;
    
    int index = HashFunc(clientfd);
    GetMutex(&mutexs[index]);
    int loc = FindInBucket(index, clientfd);
    if (loc != -1) {
	time(&(buckets[index][loc].t));
    } else {
	ret = -1;
    }
    ReleaseMutex(&mutexs[index]);

    return ret;
}


void KVEpoll::CleanDeadConnection() {
    while (1) {
	time_t now = time(NULL);
	double sec;
	vector<KVEpollNode>::iterator it;

	for (int i = 0; i < NEPOLLBUCKET; ++ i) {

	    GetMutex(&mutexs[i]);
	    it = buckets[i].end();
	    while (it != buckets[i].begin()) {
		it --;
		sec = difftime(now, (*it).t);
		if (sec >= TIMEOUTSECOND) {
		    close((*it).fd);
		    it = buckets[i].erase(it);
		}
	    }
	    ReleaseMutex(&mutexs[i]);
	}
	sleep(SECONDPERPLUSE);
    }
}

void* __CleanDeadConnection(void *arg) {
    KVEpoll *kvEpoll = (KVEpoll *)arg;
    kvEpoll->CleanDeadConnection();
    return (void *)0;
}

int KVEpoll::StartCleanDaemon() {
    pthread_t tid;
    if (pthread_create(&tid, NULL, __CleanDeadConnection, (void *)this) != 0) {
	cout << "KVEpoll::StartCleanDaemon create thread fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    }
    return 0;
}

unsigned int KVEpoll::HashFunc(int fd) {
    return fd % NEPOLLBUCKET;
}

unsigned int KVEpoll::FindInBucket(int index, int fd) {
    for (unsigned int i = 0; i < buckets[index].size(); ++ i) {
	if (buckets[index][i].fd == fd) return i;
    }
    return -1;
}

}
