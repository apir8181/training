
#include "simplekvsvr.h"
#include "threadpool.h"
#include "kvhelper.h"
#include "socket.h"
#include "kvconfig.h"
#include "kvstat.h"
#include <pthread.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <string.h>
#include <map>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

extern int errno;

using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::map;

namespace mmtraining {

////////////////////////////////////////class SimpleKVSvrWork
SimpleKVSvrWork::SimpleKVSvrWork(int clientfd, KVStore *kvStore, KVEpoll *kvEpoll) {
    clientSocket = new ClientSocket(clientfd);
    if (clientSocket == NULL) {
	cout << "SimpleKVSvrWork::SimpleKVSvrWork error: clientSocket create error"
	     << endl;
    }

    this->kvStore = kvStore;
    if (kvStore == NULL) {
	cout << "SimpleKVSvrWork::SimpleKVSvrWork error: kvStore is empty" << endl;
    }

    this->kvEpoll = kvEpoll;
    if (kvEpoll == NULL) {
	cout << "SimpleKVSvrWork::SimpleKVSvrWork error: kvEpoll is empty" << endl;
    }

    this->clientfd = clientfd;
}

SimpleKVSvrWork::~SimpleKVSvrWork() {
    if (clientSocket != NULL) {
	delete clientSocket;
    }
}

int SimpleKVSvrWork::DoWork() {
    if (clientSocket == NULL) {
	cout << "client socket has not init in thread " << pthread_self() << endl;
	return -1;
    }

    char* buf = new char[KVDATABUFSIZE + 10];
    char* res = new char[VALUEMAXSIZE + 1];


    if (clientSocket->ReadLine(buf, sizeof(buf)) == -1 ||
	(strncmp(buf, "quit", 4) == 0 && strlen(buf) == 4) || 
	strcmp(buf, "") == 0) {
	if (kvEpoll->DeleteClient(clientfd) != 0){
	    cout << "SimpleKVSvrWork::DoWork Delete from epoll error:"
		 << strerror(errno) << endl;
	}
	clientSocket->Close();

	//cout << "a connection close" << endl;
	goto end;
    } else if (strncmp(buf, "get", 3) == 0) {
	Get(buf, res);
    } else if (strncmp(buf, "set", 3) == 0) {
	Set(buf, res);
    } else if (strncmp(buf, "delete", 6) == 0) {
	Delete(buf, res);
    } else if (strcmp(buf, "stats") == 0) {
	Stats(res);
    } else {
	Prompt(res);
    }

    //cout << "receive:" << buf << strlen(buf) << endl;

    kvEpoll->UpdateClient(clientfd);
    if (clientSocket->WriteLine(res, strlen(res)) == -1) {
	cout << "return feedback to client error in thread "
	     << pthread_self() << " error" << endl;
	clientSocket->Close();
    }

 end:
    delete clientSocket;
    delete []buf;
    delete []res;
    clientSocket = NULL;

    return 0;
}

bool SimpleKVSvrWork::NeedDelete() const {
    return true;
}

void SimpleKVSvrWork::Get(char *buf, char *res) {
    string s(buf);
    stringstream ss(s, std::ios_base::in);
    
    string command, key, value;
    ss >> command >> key;
    
    if (command != "get" || key.size() == 0 || key.size() > KEYMAXSIZE) {
	strcpy(res, "get command error");
    } else if ((value = kvStore->GetValue(key)) == "") {
	strcpy(res, "key not exist");
    } else {
	strcpy(res, value.c_str());
    }

    ss.clear();
    s.clear();
}

void SimpleKVSvrWork::Set(char *buf, char *res) {
    string s(buf);
    stringstream ss(s, std::ios_base::in);
    
    string command, key, value;
    ss >> command >> key >> value;
    if (command != "set" || key.size() == 0 || key.size() > KEYMAXSIZE ||
	value.size() == 0 || value.size() > VALUEMAXSIZE) {
	strcpy(res, "set command error");
    } else {

	int ret = kvStore->WriteValue(key, value);
	if (ret == 0) {
	    strcpy(res, "OK");
	} else if (ret == -1) {
	    strcpy(res, "Write into file error");
	} else if (ret == -2) {
	    strcpy(res, "Write into index error");
	}

    }
}

void SimpleKVSvrWork::Delete(char *buf, char *res) {
    string s(buf);
    stringstream ss(s, std::ios_base::in);
    
    string command, key;
    ss >> command >> key;
    if (command != "delete" || key.size() == 0 || key.size() > KEYMAXSIZE) {
	strcpy(res, "delete command error");
    } else {

	int ret = kvStore->DeleteKey(key);
	if (ret == 0) {
	    strcpy(res, "OK");
	} else if (ret == -1) {
	    strcpy(res, "key not exist");
	} else if (ret == -2) {
	    strcpy(res, "file write error");
	}

    }
}

void SimpleKVSvrWork::Stats(char *res) {
    kvstat stat = kvStore->GetStat();
    sprintf(res, "count:%lld, mem:%lld, file:%lld, hits:%lld, misses:%lld", 
	    stat.keyCount, stat.memSize, stat.fileSize, stat.hitCount, stat.missCount);
}

void SimpleKVSvrWork::Quit(char *res) {
    strcpy(res, "OK");
}

void SimpleKVSvrWork::Prompt(char *res) {
    strcpy(res, "command: [get <key>] | [set <key> <value>] | [delete <key>] | [stats] | [quit]");
}

///////////////////////////////////////////////class SimpleKVSvr
SimpleKVSvr::SimpleKVSvr() {}

int SimpleKVSvr::Start(unsigned short port, int threadCount) {
    this->port = port;
    this->threadCount = threadCount;
    
    int err;
    if ((err = pthread_create(&daemonid, NULL, __SimpleKVSvrDaemon, (void *)this)) != 0) {
	cout << "SimpleKVSvr::Start error in pthread create: " << strerror(err) << endl;
	return -1;
    }

    return 0;
}

void SimpleKVSvr::ShutDown() {
    exit(0);
}

void SimpleKVSvr::PrintStats() {
    kvstat stat = kvStore.GetStat();
    char buf[512];
    sprintf(buf, "stats:\ncount:%lld, mem:%lld, file:%lld, hits:%lld, misses:%lld",
	    stat.keyCount, stat.memSize, stat.fileSize, stat.hitCount, stat.missCount);
    cout << buf << endl;
}

int SimpleKVSvr::DaemonRun() {
    bool isNonBlocking = true;

    if (WorkerThreadPool::Start(threadCount) == -1) {
	cout << "SimpleKVSvr::DaemonRun thread pool can not create enough thread" << endl;
	return -1;
    } else if (server.Listen(SERVERIP, port, isNonBlocking) == -1) {
	cout << "SimpleKVSvr::DaemonRun server can not listen" << endl;
	return -1;
    }


    KVEpoll kvEpoll(&server, &workQueue, &kvStore);
    kvEpoll.StartCleanDaemon();
    while (1) {
	kvEpoll.OneTimeLoop();
    }
	
    if ( !(WorkerThreadPool::Shutdown() && WorkerThreadPool::JoinAll()) ) {
	cout << "echo svr can not shutdown" << endl;
	return -1;
    }

    return 0;
}

int SimpleKVSvr::Refactor() {
    return kvStore.Refactor();
}

void *__SimpleKVSvrDaemon(void *arg) {
    SimpleKVSvr* server = (SimpleKVSvr *)(arg);
    server->DaemonRun();
    return (void *)0;
}

}
