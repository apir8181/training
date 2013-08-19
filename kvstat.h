
#ifndef MMTRAINING_KVSTAT_H
#define MMTRAINING_KVSTAT_H

#include "kvconfig.h"

namespace mmtraining {

struct kvstat {
    long long keyCount;
    long long memSize;
    OFFSETTYPE fileSize;
    long long hitCount, missCount;
};

}

#endif
