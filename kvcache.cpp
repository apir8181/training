#include "kvcache.h"
#include "kvhelper.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <time.h>

using std::cout;
using std::endl;
using std::string;

namespace mmtraining {

KVCache::KVCache() {
    int err;

    buckets = new NodeVec[NCACHEBUCKET];
    locks = new pthread_mutex_t[NCACHEBUCKET];

    if (buckets == NULL || locks == NULL) {
	cout << "KVCache::KVCache malloc error" << endl;
	cout << "Exit" << endl;
	exit(1);
    }

    for (int i = 0; i < NCACHEBUCKET; ++ i) {
	hitCount[i] = missCount[i] = 0;
	buckets[i].clear();
	if ((err = pthread_mutex_init(&locks[i], NULL)) != 0) {
	    cout << "KVCache::KVCache thread mutex init errror: " 
		 << strerror(err) << endl;
	    cout << "Exit" << endl;
	    exit(1);
	}
    }
}

KVCache::~KVCache() {
    for (int i = 0; i < NCACHEBUCKET; ++ i) {
	buckets[i].clear();
	pthread_mutex_destroy(&locks[i]);
    }

    delete []buckets;
    delete []locks;
}

int KVCache::Get(string &key, string &value) {
    int index = HashFunc(key);
    int loc, ret = -1;

    GetMutex(&locks[index]);
    loc = Find(index, key);
    if (loc != -1) {
	++ hitCount[index];
	ret = 0;
	value = buckets[index][loc].value;
	buckets[index][loc].t = time(NULL);
    } else {
	++ missCount[index];
	value = "";
    }
    ReleaseMutex(&locks[index]);

    return ret;
}

void KVCache::Set(string &key, string &value) {
    int index = HashFunc(key);
    int loc;

    GetMutex(&locks[index]);
    loc = Find(index, key);
    if (loc == -1) { // cache not find

	if (buckets[index].size() == NBUCKETSIZE) { // bucket is full

	    int pos = 0;
	    time_t min = buckets[index][0].t;

	    for (unsigned int i = 1; i < buckets[index].size(); ++ i) {
		if (buckets[index][i].t < min) {
		    min = buckets[index][i].t;
		    pos = i;
		}
	    }
	    
	    buckets[index][pos].key = key;
	    buckets[index][pos].value = value;
	    buckets[index][pos].t = time(NULL);

	} else { // bucket is not full
	    buckets[index].push_back(KVCacheNode(key, value));
	}
	
    } else { // find in cache, just overwrite it
	buckets[index][loc].value = value;
	buckets[index][loc].t = time(NULL);
    }

    ReleaseMutex(&locks[index]);
}

void KVCache::Delete(string &key) {
    int index = HashFunc(key);
    int loc;

    GetMutex(&locks[index]);
    loc = Find(index, key);
    if (loc != -1) {
	buckets[index].erase(buckets[index].begin() + loc);
    }
    ReleaseMutex(&locks[index]);
}

int KVCache::Find(int index, string &key) {
    int ret = -1;
    unsigned int s = buckets[index].size();
    for (unsigned int i = 0; i < buckets[index].size(); ++ i) {
	if (buckets[index][i].key == key) {
	    ret = i;
	    break;
	}
    }
    return ret;
}

unsigned int KVCache::HashFunc(string &key) {
    return JenkinsHash(key) % NCACHEBUCKET;
}

void KVCache::GetStat(kvstat &stat) {
    for (int i = 0; i < NCACHEBUCKET; ++ i) {
	GetMutex(&locks[i]);
	stat.hitCount += hitCount[i];
	stat.missCount += missCount[i];
	ReleaseMutex(&locks[i]);
    }
}

}////////////////////////////////namespace KVCACHE
