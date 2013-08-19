
#include "kvstore.h"
#include "kvconfig.h"
#include "kvhelper.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysinfo.h>

using std::string;
using std::cout;
using std::endl;

extern int errno;

namespace mmtraining {

KVStore::KVStore() {
    kvFile = new KVFile(FILENAME);
    kvIndex = new KVIndex();

    if (kvFile == NULL || kvIndex == NULL) {
	cout << "KVStore::KVStore can not new kvfile or kvIndex" << endl;
	cout << "Exit" << endl;
	exit(1);
    } else if (kvIndex->Build(kvFile) == -1) {
	cout << "KVStore::KVStore build kvindex fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    } else if (pthread_rwlock_init(&refactorLock, NULL) != 0) {
	cout << "KVStore::KVStore read write lock init fail" << endl;
	cout << "Exit" << endl;
	exit(1);
    }
    isRefactor = false;
}


int KVStore::WriteValue(string &key, string &value) {
    if (isRefactor) return ISREFACTOR;
    int ret = 0;

    GetReadLock(&refactorLock);    
    OFFSETTYPE off = kvFile->WritePairAppend(key, value);
    if (off == -1) {
	ret = -1;
	goto end;
    }
    kvCache.Set(key, value);
    if (kvIndex->SetFileOff(key, off) == -1) {
	cout << "KVStore::SetValue error in kvIndex.SetFileOff in thread "
	     << pthread_self() << ":" << strerror(errno) << endl;
	ret = -2;
    }

 end:
    ReleaseReadLock(&refactorLock);

    return ret;
}


string KVStore::GetValue(string &key) {
    string ret = "";
    
    if (kvCache.Get(key, ret) == -1) {
	GetReadLock(&refactorLock);
	OFFSETTYPE off = kvIndex->GetFileOff(key);
	if (off == -1) {}
	else if (kvFile->ReadPair(key, ret, off) == -1) {
	    cout << "KVStore::GetValue error in reading kvFile in thread " 
		 << pthread_self() << ":" << strerror(errno) << endl;
	} else {
	    kvCache.Set(key, ret);
	}
	ReleaseReadLock(&refactorLock);
    }

    return ret;
}

int KVStore::DeleteKey(string &key) {
    if (isRefactor) return ISREFACTOR;

    int ret = 0;
    kvCache.Delete(key);
    GetReadLock(&refactorLock);
    int err = kvIndex->DeleteKey(key);

    if (err == -1) {
	ret = -1;
    } else {
	string value = "";
	if (ret == 0 && kvFile->WritePairAppend(key, value) < 0) {
	    ret = -2;
	}
    }
    ReleaseReadLock(&refactorLock);

    return ret;
}

int KVStore::Refactor() {
    if (isRefactor) return ISREFACTOR;

    KVFile* newFile = new KVFile(NEWFILENAME);
    KVIndex* newIndex = new KVIndex();
    int ret = 0;

    if (newFile == NULL) {
	cout << "KVStore::Refactor not enough memory" << endl;
	ret = -1;
    } else if (newIndex == NULL) {
	cout << "KVStore::Refactor not enough memory" << endl;
	ret = -2;
    }

    cout << "KVStore::Refactor start" << endl;

    int count = 0;
    kvIndex->InitRefactor();
    while (kvIndex->GetNextRefactor(kvFile, newFile, newIndex) == 0) {
	cout << "\t" << ++ count << endl;
    }
    cout << "KVStore::Refactor end" << endl;
    
    GetWriteLock(&refactorLock);
    delete kvFile;
    delete kvIndex;
    kvFile = newFile;
    kvIndex = newIndex;
    if (remove(FILENAME) != 0) {
	cout << "KVStore::Refactor delete old file error:" << strerror(errno) << endl;
	ret = -1;
    } else if (kvFile->Rename(FILENAME) != 0) {
	cout << "KVStore::Refactor rename new file error:" << strerror(errno) << endl;
	ret = -2;
    } else {
	isRefactor = false;
    }
    ReleaseWriteLock(&refactorLock);

    return ret;
}

kvstat KVStore::GetStat() {
    kvstat stat;
    stat.keyCount = stat.memSize = stat.fileSize = stat.hitCount = stat.missCount = 0;

    GetReadLock(&refactorLock);
    kvIndex->GetStat(stat);
    kvCache.GetStat(stat);
    kvFile->GetStat(stat);
    GetMemoryStat(stat);
    ReleaseReadLock(&refactorLock);

    return stat;
}

void KVStore::GetMemoryStat(kvstat &stat) {
    FILE* file = fopen("/proc/self/status", "r");
    int temp;
    char buf[128];
    char *line = buf;

    while (fgets(buf, 128, file) != NULL) {
	if (strncmp(buf, "VmRSS:", 6) == 0) {
	    temp = strlen(buf);
	    while (line[0] < '0' || line[0] > '9') {
		line ++;
	    }

	    line[temp - 3] = '\0';
	    temp = atoi(line);
	    break;
	}
    }

    fclose(file);
    stat.memSize = (long long)(temp) * 1024;
}

}///////////////////////////////namespace mmtraining

