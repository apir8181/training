
#ifndef MMTRAINING_KVFILE_H
#define MMTRAINING_KVFILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include "kvconfig.h"
#include "kvstat.h"

namespace mmtraining {

class KVFile {
public:
    
    KVFile(const char* filename);

    ~KVFile();

    /**
     * @return: 0 success, < 0 false
     */
    int ReadSeek(OFFSETTYPE off);

    /**
     * key, value will be change in this method
     * @return: 0 success, < 0 false
     * @warning: it can not read empty key, value pair
     */
    int ReadPair(std::string &key, std::string &value, OFFSETTYPE off);
    
    /**
     * same as above, but just read pair ahead
     * @return: 0 success, < 0 false
     * @warning: only used in kvindex.Build
     */
    int ReadPairAhead(std::string &key, std::string &value);

    /**
     * @precond: key should not be ""
     * @return: the offset it writes, it return negative number if fail
     * 
     */
    OFFSETTYPE WritePairAppend(std::string &key, std::string &value);

    bool Eof();

    OFFSETTYPE GetOff();

    /**
     * need to add lock here
     */
    void GetStat(kvstat& stat);

    /**
     * @precond: str should not be null
     * rename the file
     */
    int Rename(const char *str);

private:

    pthread_rwlock_t lock;
    FILE *pFile;
    char filename[1024];

    OFFSETTYPE writeOff;
};

}
#endif
