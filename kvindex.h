
#ifndef MMTRAINING_KVINDEX_H
#define MMTRAINING_KVINDEX_H

#include "kvfile.h"
#include "kvconfig.h"
#include "memorystore.h"
#include <string>
#include <map>
#include <pthread.h>

using std::string;
using std::map;

namespace mmtraining {

struct KVIndexNode {
    string key;
    OFFSETTYPE offset;
    KVIndexNode(string &k, OFFSETTYPE off) {
	key = k;
	offset = off;
    }
};

class KVIndex {
public:

    KVIndex();
    ~KVIndex();

    int Build(KVFile *file);

    OFFSETTYPE GetFileOff(string& key);

    int SetFileOff(string& key, OFFSETTYPE offset);

    int DeleteKey(string &key);
    
    void PrintStoreMessage();

    void GetStat(kvstat& stat);

    /**
     * @warning: only one method can call this method
     */
    void InitRefactor();
    /**
     * @warning: only one method can call this method
     */
    int GetNextRefactor(KVFile* oldFile, KVFile *newFile, KVIndex *newIndex);

private:
    unsigned int HashFunc(string &key);
    
    StlMap *buckets;
    pthread_rwlock_t *locks;

    
    int nowIndex;
};

}

#endif
