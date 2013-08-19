
#include "kvindex.h"
#include "kvconfig.h"
#include "kvfile.h"
#include "kvhelper.h"
#include "memorystore.h"
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <map>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

using std::string;
using std::cout;
using std::endl;
using std::map;

namespace mmtraining {

KVIndex::KVIndex() {
    buckets = new StlMap[NINDEXBUCKET];
    locks = new pthread_rwlock_t[NINDEXBUCKET];

    if (buckets == NULL || locks == NULL) {
	cout << "KVIndex::KVIndex malloc error" << endl;
	cout << "Exit" << endl;
	exit(1);
    }

    int err;
    for (int i = 0; i < NINDEXBUCKET; ++ i) {
	buckets[i].ClearAll();
	if ((err = pthread_rwlock_init(&locks[i], NULL)) != 0) {
	    cout << "KVIndex::KVIndex pthread lock init error: " 
		 << strerror(err) << endl;
	    cout << "Exit" << endl;
	    exit(1);
	}
    }
}

KVIndex::~KVIndex() {
    for (int i = 0; i < NINDEXBUCKET; ++ i) {
	buckets[i].ClearAll();
	pthread_rwlock_destroy(&locks[i]);
    }
    delete []buckets;
    delete []locks;
}


int KVIndex::Build(KVFile* kvFile) {
    if (kvFile == NULL) {
	return -1;
    }

    kvFile->ReadSeek(0);

    cout << "KVIndex::Build start building index" << endl;

    int count = 0;

    while (!kvFile->Eof()) {
	string key = "", value = "";
	OFFSETTYPE off = kvFile->GetOff();

	if (kvFile->ReadPairAhead(key, value) == 0) {
	    if (value != "") { 
		SetFileOff(key, off);
	    } else { //if value is "", then it means it has been deleted
		DeleteKey(key);
	    }
	}

	++ count;
	if (count % 100000 == 0) cout << count << endl;
    }

    cout << "KVIndex::Build building index finish" << endl;

    return 0;
}

OFFSETTYPE KVIndex::GetFileOff(std::string &key) {
    int index = HashFunc(key);
    GetReadLock(&locks[index]);
    OFFSETTYPE ret = buckets[index].Get(key);
    ReleaseReadLock(&locks[index]);
    return ret;
}

int KVIndex::SetFileOff(std::string &key, OFFSETTYPE offset) {
    int index = HashFunc(key);
    GetWriteLock(&locks[index]);
    int ret = buckets[index].Set(key, offset);
    ReleaseWriteLock(&locks[index]);
    return ret;
}

int KVIndex::DeleteKey(std::string &key) {
    int index = HashFunc(key);
    GetWriteLock(&locks[index]);
    int ret =  buckets[index].Delete(key);
    ReleaseWriteLock(&locks[index]);
    return ret;
}

void KVIndex::PrintStoreMessage() {
    for (int i = 0; i < NINDEXBUCKET; ++ i) {
	cout << "KVIndex::PrintStoreMessage print index " << i << endl;
	GetReadLock(&locks[i]);
	buckets[i].PrintAll();
	ReleaseReadLock(&locks[i]);
    }
}

void KVIndex::GetStat(kvstat& stat) {
    for (int i = 0; i < NINDEXBUCKET; ++ i) {
	GetReadLock(&locks[i]);
	stat.keyCount += buckets[i].Size();

	ReleaseReadLock(&locks[i]);
    }
}

unsigned int KVIndex::HashFunc(string &key) {
    return JenkinsHash(key) % NINDEXBUCKET;
}

void KVIndex::InitRefactor() {
    nowIndex = 0;
}

int KVIndex::GetNextRefactor(KVFile* oldFile, KVFile *newFile, KVIndex *newIndex) {
    if (nowIndex == NINDEXBUCKET) return -1;
    
    string key, value;
    OFFSETTYPE off;
    int ret = 0;

    buckets[nowIndex].ClearIterator();
    while (1) {
	buckets[nowIndex].GetNextIterator(key, off);
	if (key == "") break;

	if (oldFile->ReadPair(key, value, off) != 0) {
	    cout << "KVIndex::GetNextRefactor read old file pair error" << endl;
	    ret = -2;
	    break;
	} else if ((off = newFile->WritePairAppend(key, value)) < 0) {
	    cout << "KVIndex::GetNextRefactor write new file pair error" << endl;
	    ret = -3;
	    break;
	} else if (newIndex->SetFileOff(key, off) != 0) {
	    cout << "KVindex::GetNextRefactor write new index error" << endl;
	    ret = -4;
	    break;
	}
    }

    ++ nowIndex;

    return ret;
}

}//namespace mmtraining


