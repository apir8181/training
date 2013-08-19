
#include "socket.h"
#include "threadpool.h"
#include "kvhelper.h"
#include "kvconfig.h"
#include <string.h>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <algorithm>

using namespace mmtraining;
using namespace std;

string* key = new string[NTESTDATA];
int keyCount = 0;
pthread_mutex_t keyMutex;

pthread_mutex_t mutex;
int num = 0;

void init_key() {
    cout << "sort key start" << endl;
    sort(key, key + NTESTDATA);
    cout << "sort key end" << endl;
}

void delete_sprintf(char *buf) {
    GetMutex(&keyMutex);
    if (keyCount != NTESTDATA) {
	sprintf(buf, "delete %s", key[keyCount ++].c_str());
    } else {
	sprintf(buf, "");
    }
    ReleaseMutex(&keyMutex);
}

class TestThread : public Thread {
public:
    TestThread(unsigned short port, void (*func)(char *buf)) {
	this->port = port;
	this->func = func;
	isFinish = false;
    }

    int Run() {
	int count = 0;
	ClientSocket socket;
	char *buf = new char[KVDATABUFSIZE];

	if (socket.Connect(SERVERIP, port) != 0) goto end;
	sleep(1);
	
	while (1) {
	    func(buf);
	    if (keyCount == NTESTDATA) break;

	    if (socket.WriteLine(buf, strlen(buf)) < 0) {
		goto end;
	    } 
	    memset(buf, 0, sizeof(buf));
	    if (socket.ReadLine(buf, sizeof(buf)) < 0) {
		goto end;
	    } else if (strcmp(buf, "OK") != 0) {
		cout << "readline error read:" << buf << endl;
		goto end;
	    }

	    if (strlen(buf) == 0) continue;
	    ++ count;
	    if (count == PERITERTIME) {
		count = 0;
		GetMutex(&mutex);
		num += PERITERTIME;
		ReleaseMutex(&mutex);
	    }
	}

    end:
	socket.Close();
	isFinish = true;
	delete []buf;

	GetMutex(&mutex);
	num += count;
	ReleaseMutex(&mutex);

	return 0;
    }

    bool isFinish;

private:
    void (*func)(char *buf);
    unsigned short port;
};

vector<TestThread*> create_threads(unsigned short port, int threadCount,
				      void (*func)(char *)) {
    vector<TestThread*> threads;
    threads.clear();

    for (int i = 0; i < threadCount; ++ i) 
	threads.push_back(new TestThread(port, func));
				      

    for (int i = 0; i < threadCount; ++ i) {
	threads[i]->Start();
	usleep(1);
    }
    
    return threads;
}

void update_threads(unsigned short port, int threadCount, void (*func)(char *),
		   vector<TestThread*> &threads) {
    vector<TestThread*>::iterator it = threads.end();
    while (it != threads.begin()) {
	it --;
	TestThread* thread = *it;
	if (thread->isFinish) {
	    thread->Join();
	    delete thread;
	    it = threads.erase(it);
	}
    }

    for (int i = 0; i < threadCount - threads.size(); ++ i) {
	TestThread* thread = new TestThread(port, func);
	if (thread->Start() != 0) delete thread;
	else threads.push_back(thread);
    }
}


void test(unsigned short port, int count) {
    vector<TestThread *> threads = create_threads(port, count, delete_sprintf);

    int time = -1;
    while (time < TESTTIME && keyCount != NTESTDATA) {
	update_threads(port, count, delete_sprintf, threads);
	sleep(1);

	time += 1;
	GetMutex(&mutex);
	cout << "Time: " << time << " Count:" << num << " Speed:" << 
	    (float)num / time << endl;
	ReleaseMutex(&mutex);
    }
    
    GetMutex(&mutex);
    cout << "DeleteClient Time: 60s Count:" << num << " Speed:" << (float)num / time 
	 << endl;
    ReleaseMutex(&mutex);
}

void load_data() {
    cout << "start loading data" << endl;

    ifstream fin;
    fin.open(TESTFILENAME);
    for (int i = 0; i < NTESTDATA; ++ i) {
	fin >> key[i];
	if (i % 100000 == 0) cout << i << endl;
    }
    fin.close();

    cout << "loading data end" << endl;
}


int main(int argc, char **argv) {
    if (argc < 3) {
	cout << "usage: " << argv[0] << " port count" << endl;
	return 1;
    }

    unsigned short port = atoi(argv[1]);
    int count = atoi(argv[2]);
 
    srand((unsigned)time(NULL));

    load_data();
    init_key();
    test(port, count);

    return 0;
}
