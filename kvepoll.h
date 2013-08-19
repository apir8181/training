
#ifndef MMTRAINING_KVEPOLL_H
#define MMTRAINING_KVEPOLL_H
#include "socket.h"
#include "kvstore.h"
#include "threadpool.h"
#include <sys/epoll.h>
#include <vector>
#include <time.h>
#include <pthread.h>

using std::vector;

namespace mmtraining {

struct KVEpollNode {
    int fd;
    time_t t;
    KVEpollNode(int id) {
	fd = id;
	time(&t);
    }
};

class KVEpoll {

public:
    /**
     * @precond: all parms should not be NULL
     */
    KVEpoll(ServerSocket *socket, WorkQueue *queue, KVStore *kvStore);
    ~KVEpoll();

    int OneTimeLoop();

    void AddClient(int clientfd);
    int DeleteClient(int clientfd);
    int UpdateClient(int clientfd);

    int StartCleanDaemon();
    /**
     * Daemon thread method, it is used for cleaning up daed connection.
     * Pulse Rast is SECONDPERPLUSE
     */
    void CleanDeadConnection();

private:
    int ServerFunc();
    int ClientFunc(struct epoll_event event);



    unsigned int HashFunc(int fd);
    unsigned int FindInBucket(int index, int fd);

    vector<KVEpollNode> *buckets;
    pthread_mutex_t *mutexs;

    int serverfd;
    int epollfd;
    struct epoll_event *events;

    ServerSocket *server;
    WorkQueue *queue;
    KVStore *kvStore;
};

}

#endif
