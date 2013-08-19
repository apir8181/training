
#ifndef MMTRAINING_KVSTORE_H
#define MMTRAINING_KVSTORE_H

#include <string>
#include <pthread.h>
#include <map>
#include "kvindex.h"
#include "kvfile.h"
#include "kvstat.h"
#include "kvcache.h"

using std::string;

namespace mmtraining {

class KVStore {
public:
    
    /**
     * This constructor is call in the main process before it creates any thread.
     */
    KVStore();

    /**
     * thread method
     * @precond: key && value not empty
     * @return: 0 sucess, -1 file write error, -2 index set error
     */
    int WriteValue(string &key, string &value);

    /**
     * thread method
     * @precond: key not empty
     * @return: "": none exist object, string: success
     */
    string GetValue(string &key);

    /**
     * @precond: key not empty
     * @return: 0: success, -1 no such key, -2 file write error
     */
    int DeleteKey(string &key);

    kvstat GetStat();

    /**
     * @precond: kvIndex has been built
     * @return: 0: success, -1 fail
     */
    int Refactor();

private:

    void GetMemoryStat(kvstat &stat);

    KVFile* kvFile;
    KVIndex* kvIndex;
    KVCache kvCache;

    pthread_rwlock_t refactorLock;
    bool isRefactor;
};


}
#endif
