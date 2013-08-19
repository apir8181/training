
#ifndef MMTRANING_KVCACHE_H
#define MMTRANING_KVCACHE_H

#include <string>
#include <vector>
#include <pthread.h>
#include <time.h>
#include "kvconfig.h"
#include "kvstat.h"

using std::string;
using std::vector;

namespace mmtraining {

struct KVCacheNode {
    string key, value;
    time_t t;
    KVCacheNode(string &k, string &v) {
	key = k;
	value = v;
	t = time(NULL);
    }
};

class KVCache {
public:
    KVCache();
    ~KVCache();
    
    /**
     * @parms: key, value(value is to be changed)
     * @return: 0 success, -1 not exist
     */
    int Get(string &key, string &value);

    void Set(string &key, string &value);
    
    void Delete(string &key);

    void GetStat(struct kvstat &stat);

private:
    unsigned int HashFunc(string &key);

    /**
     * Find key in the index th bucket
     * @return: index: found, -1: not found
     */
    int Find(int index, string &key);

    typedef vector<KVCacheNode> NodeVec;

    /**
     * For each buckets, the maximum size is 10
     */
    NodeVec* buckets;
    pthread_mutex_t* locks;

    long long hitCount[NCACHEBUCKET], missCount[NCACHEBUCKET];
};

}
#endif
