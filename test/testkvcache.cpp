
#include "kvcache.h"
#include "kvhelper.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

using namespace std;
using namespace mmtraining;

void testGet() {
    KVCache cache;
    string key, value, tempVal;
    genRandomStr(key, 8);
    genRandomStr(value, 100);
    cache.Set(key, value);
    cache.Get(key, tempVal);
    assert(value == tempVal);
}

void testDelete() {
    KVCache cache;
    string key, value;
    genRandomStr(key, 10);
    genRandomStr(value, 100);
    cache.Set(key, value);
    cache.Delete(key);
    assert(cache.Get(key, value) == -1);
}

int main(int argc, char **argv) {
    srand((unsigned)time(NULL));
    testGet();
    testDelete();
    return 0;
}
