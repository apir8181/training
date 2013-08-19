
#include "kvfile.h"
#include "kvconfig.h"
#include <iostream>
#include <string>
#include <assert.h>

using namespace std;
using namespace mmtraining;

void test() {
    KVFile file(FILENAME);
    string key = "haha", value = "hehe";

    OFFSETTYPE off = file.WritePairAppend(key, value);
    assert(off >= 0);
    
    string tempKey, tempVal;
    assert(file.ReadPair(tempKey, tempVal, off) == 0);
    assert(key == tempKey && value == tempVal);
}

int main(int argc, char **argv) {
    test();
    return 0;
}
