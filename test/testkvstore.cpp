
#include "../kvstore.h"
#include "../kvindex.h"
#include "../kvcache.h"
#include "../kvstat.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>

using namespace std;
using namespace mmtraining;

/**
 * Remember to gen kvdatabase before calling this program
 */
int main() {
    KVStore store;
    string command, key, value;

    while (1) {
	cout << endl;
	cout << "waiting for command: get key | write key value | delete key " 
	     << "| stats | refactor | exit" << endl;
	cout << endl;

	cin >> command; 
	if (command == "write") {
	    cin >> key >> value;
	    store.WriteValue(key, value);
	} else if (command == "get") {
	    cin >> key;
	    cout << "getValue: " << store.GetValue(key) << endl;
	} else if (command == "delete") {
	    cin >> key;
	    cout << "deleteKey: " << store.DeleteKey(key) << endl;
	} else if (command == "stats" ) {
	    kvstat stat = store.GetStat();
	    cout << "stats: <keyCount>" << stat.keyCount << " <memUsed>" << stat.memSize
		 << " <fileSize>" << stat.fileSize << " <hitCount>" << stat.hitCount
		 << " <missCount>" << stat.missCount << endl;
	} else if (command == "refactor") {
	    store.Refactor();
	} else if (command == "exit") {
	    exit(0);
	} else {
	    cout << "Type wrong, type again" << endl;
	    continue;
	}

	//store.PrintKVIndexInfo();
	//store.PrintKVCacheInfo();
    }

    return 0;
}
