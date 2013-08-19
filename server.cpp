
#include "simplekvsvr.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace mmtraining;

using std::cin;
using std::cout;
using std::endl;

int main(int argc, char **argv) {
    if (argc < 3) {
	cout << "<usage> " << argv[0] << " port threadCount" << endl;
	return 1;
    }

    unsigned short port = atoi(argv[1]);
    int threadCount = atoi(argv[2]);

    SimpleKVSvr server;
    if (server.Start(port, threadCount) == -1) {
	cout << "main thread can not start server " << endl;
	return 2;
    }

    while (1) {
	string str;
	cin >> str;

	if (str == "shutdown") {
	    server.ShutDown();
	    break;
	} else if (str == "stats") {
	    server.PrintStats();
	} else if (str == "refactor") {
	    server.Refactor();
	} else {
	    cout << "invalid command: shutdown | stats | refactor" << endl;
	}
    }

    return 0;
}
