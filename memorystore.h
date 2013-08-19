
#ifndef MMTRAINING_MEMORYSTORE_H
#define MMTRAINING_MEMORYSTORE_H

#include <map>
#include <string>
#include "kvconfig.h"

using std::string;
using std::map;

namespace mmtraining {

class MemoryStore {
public:
    virtual int Set(string& str, OFFSETTYPE offset) = 0;
    virtual OFFSETTYPE Get(string& str) = 0;
    virtual int Delete(string &str) = 0;
    virtual void PrintAll() = 0;
    virtual void ClearAll() = 0;
    virtual long long Size() = 0;
    virtual void ClearIterator() = 0;
    virtual void GetNextIterator(string &key, OFFSETTYPE& off) = 0;
};

class StlMap : public MemoryStore {
public:
    StlMap();
    ~StlMap();
    virtual int Set(string& key, OFFSETTYPE offset);
    virtual OFFSETTYPE Get(string& key);
    virtual int Delete(string &key);
    virtual void PrintAll();
    virtual void ClearAll();
    virtual long long Size();
    virtual void ClearIterator();
    virtual void GetNextIterator(string &key, OFFSETTYPE& off);
private:
    map<std::string, OFFSETTYPE> m;
    map<std::string, OFFSETTYPE>::iterator mit;
};


}
#endif
