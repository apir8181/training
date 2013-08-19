
#include "kvhelper.h"
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

extern int errno;

using std::cout;
using std::endl;
using std::string;

void GetMutex(pthread_mutex_t *mutex) {
    int err = 0;
    if ((err = pthread_mutex_lock(mutex)) != 0) {
	cout << "GetMutex: thread " << pthread_self() << " get mutex fail :"
	     << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

void ReleaseMutex(pthread_mutex_t *mutex) {
    int err = 0;
    if ((err = pthread_mutex_unlock(mutex)) != 0) {
	cout << "ReleaseMutex: thread " << pthread_self() << " release mutex fail:"
	     << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

void GetReadLock(pthread_rwlock_t *lock) {
    int err = 0;
    if ((err = pthread_rwlock_rdlock(lock)) != 0) {
	cout << "GetReadLock: thread " << pthread_self() << " get fail:"
	     << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

void ReleaseReadLock(pthread_rwlock_t *lock) {
    int err = 0;
    if ((err = pthread_rwlock_unlock(lock)) != 0) {
	cout << "ReleaseReadLock: thread " << pthread_self() << " release fail"
	     << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

void GetWriteLock(pthread_rwlock_t *lock) {
    int err = 0;
    if ((err = pthread_rwlock_wrlock(lock)) != 0) {
	cout << "GetWriteLock: thread " << pthread_self() << " get fail:"
	     << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

void ReleaseWriteLock(pthread_rwlock_t *lock) {
    int err = 0;
    if ((err = pthread_rwlock_unlock(lock)) != 0) {
	cout << "ReleaseWriteLock: thread " << pthread_self() << " release fail:"
	     << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

int SetNonBlocking(int fd) {
    int opt = fcntl(fd, F_GETFL);
    int ret = 0;

    if (opt < 0) {
	cout << "SetNonBlocking fcntl F_GETFL fail" << strerror(errno) << endl;
	ret = -1;
	goto end;
    } 

    opt |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opt) < 0) {
	cout << "SetNonBlocking fcntl F_SETFL fail" << strerror(errno) << endl;
	ret = -2;
	goto end;
    }

 end:
    return ret;
}

unsigned int JenkinsHash(string &key) {
    unsigned int hash = 0;
    unsigned int size = key.size();
    for (unsigned int i = 0; i < size; ++ i) {
	hash += key[i];
	hash += (hash << 10);
	hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

void genRandomStr(string &key, int size) {
    for (int i = 0; i < size; i ++) {
	key += (char)(rand() % 26 + 'a');
    }
}
