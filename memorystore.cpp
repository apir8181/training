
#include "memorystore.h"
#include "kvconfig.h"
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>

namespace mmtraining {
//////////////////////////////class StlMap
StlMap::StlMap() {
    m.clear();
}

StlMap::~StlMap() {
    m.clear();
}

int StlMap::Set(string& key, OFFSETTYPE offset) {
    m[key] = offset;
    return 0;
}

OFFSETTYPE StlMap::Get(string& key) {
    std::map<std::string, OFFSETTYPE>::iterator it = m.find(key);
    if (it == m.end()) {
	return -1;
    }
    
    return it->second;
}    

int StlMap::Delete(string &key) {
    int ret = -1;
    std::map<std::string, OFFSETTYPE>::iterator it = m.find(key);
    if (it != m.end()) {
	m.erase(it);
	ret = 0;
    }

    return ret;
}

void StlMap::PrintAll() {
    std::cout << "StlMap::PrintAll" << std::endl;
	
    std::map<std::string, OFFSETTYPE>::iterator it = m.begin();
    while (it != m.end()) {
	std::cout << "\t" << it->first << ", " << it->second << std::endl;
	++ it;
    }
}

void StlMap::ClearAll() {
    m.clear();
    mit = m.end();
}

long long StlMap::Size() {
    return m.size();
}

void StlMap::ClearIterator() {
    mit = m.begin();
}

void StlMap::GetNextIterator(string &key, OFFSETTYPE& off) {
    if (mit != m.end()) {
	key = mit->first;
	off = mit->second;
	++ mit;
    } else {
	key = "";
	off = -1;
    }
}


}
