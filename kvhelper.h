
#ifndef MMTRAINING_KVHELPER_H
#define MMTRAINING_KVHELPER_H

#include <pthread.h>
#include <string>

void GetMutex(pthread_mutex_t *mutex);
void ReleaseMutex(pthread_mutex_t *mutex);

void GetReadLock(pthread_rwlock_t *lock);
void ReleaseReadLock(pthread_rwlock_t *lock);

void GetWriteLock(pthread_rwlock_t *lock);
void ReleaseWriteLock(pthread_rwlock_t *lock);


int SetNonBlocking(int fd);

/**
 * Jenkins One At A Time Hash: http://en.wikipedia.org/wiki/Jenkins_hash_function
 */
unsigned int JenkinsHash(std::string &key);

void genRandomStr(std::string &str, int size);

#endif
