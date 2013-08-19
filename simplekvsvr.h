
#ifndef MMTRAINING_SIMPLEKVSVR_H
#define MMTRAINING_SIMPLEKVSVR_H

#include "socket.h"
#include "threadpool.h"
#include "kvepoll.h"
#include "runnable.h"
#include "kvstore.h"

namespace mmtraining {

class SimpleKVSvrWork : public Work {
public:

    /**
     * @precond: clientfd is valid
     */
    SimpleKVSvrWork(int clientfd, KVStore *kvStore, KVEpoll *kvEpoll);
    virtual ~SimpleKVSvrWork();
    /**
     * @warning: not allow empty inputs
     */
    virtual int DoWork();
    virtual bool NeedDelete() const;
private:

    void Get(char *buf, char *res);
    void Set(char *buf, char *res);
    void Delete(char *buf, char *res);
    void Stats(char *res);
    void Quit(char *res);
    void Prompt(char *res);

    ClientSocket *clientSocket;
    KVStore *kvStore;
    KVEpoll *kvEpoll;
    int epollfd;
    int clientfd;
};

void *__SimpleKVSvrDaemon(void *arg);

class SimpleKVSvr : private WorkerThreadPool {
public:

    SimpleKVSvr();

    /**
     * @return: 0 success, -1 fail
     */
    int Start(unsigned short port, int threadCount);

    /**
     * this method should be run in daemon
     */
    int DaemonRun();

    /**
     * shutdown all connection and threads
     */
    void ShutDown();

    int Refactor();

    void PrintStats();

private:

    unsigned short port;
    int threadCount;

    pthread_t daemonid;

    ServerSocket server;
    KVStore kvStore;
};

}

#endif
