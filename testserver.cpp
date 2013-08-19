
#include "socket.h"
#include "threadpool.h"
#include "processpool.h"
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

using namespace mmtraining;
using namespace std;

#define NKEY 100000
#define PERITERTIME 5

string setValue;
void initSetValue() {
    setValue = "";
    for (int i = 0; i < 1000; ++ i)
	setValue += (char)('a' + rand() % 26);
}


string key[NKEY];

pthread_mutex_t mutex;
int num = 0;

void get_sprintf(char *buf) {
    if (rand() % 100 < 5) {
	sprintf(buf, "get %d", 123);
    } else {
	sprintf(buf, "get %s", key[rand() % NKEY].c_str());
    }
}

void set_sprintf(char *buf) {
    string s = "";
    for (int i = 0; i < 8; ++ i)
	s += (char)('a' + rand() % 26);

    sprintf(buf, "set %s %s", s.c_str(), setValue.c_str());
}

void delete_sprintf(char *buf) {
    sprintf(buf, "delete %s", key[rand() % NKEY].c_str());
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
	if (socket.Connect(SERVERIP, port) != 0) goto end;

	sleep(1);
	char buf[KVDATABUFSIZE];
	
	while (1) {
	    func(buf);

	    if (socket.WriteLine(buf, strlen(buf)) < 0) {
		goto end;
	    } 

	    memset(buf, 0, sizeof(buf));
	    if (socket.ReadLine(buf, sizeof(buf)) < 0) {
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


void test(unsigned short port, int getCount, int setCount, int deleteCount) {
    vector<TestThread *> getThreads = create_threads(port, getCount, get_sprintf);
    vector<TestThread *> setThreads = create_threads(port, setCount, set_sprintf);
    vector<TestThread *> deleteThreads = create_threads(port, deleteCount, delete_sprintf);

    int time = 0;
    while (1) {
	update_threads(port, getCount, get_sprintf, getThreads);
	update_threads(port, setCount, set_sprintf, setThreads);
	update_threads(port, deleteCount, delete_sprintf, deleteThreads);

	sleep(1);
	time += 1;
	GetMutex(&mutex);
	cout << time << " Count:" << num << endl;
	ReleaseMutex(&mutex);
    }
}


int main(int argc, char **argv) {
    if (argc < 5) {
	cout << "usage: " << argv[0] << " port getCount setCount deleteCount" << endl;
	return 1;
    }

    unsigned short port = atoi(argv[1]);
    int getCount = atoi(argv[2]), setCount = atoi(argv[3]), deleteCount = atoi(argv[4]);

    cout << "start loading data" << endl;

    ifstream fin;
    fin.open(TESTFILENAME);
    for (int i = 0; i < NKEY; ++ i) {
	fin >> key[i];
	if (i % 100000 == 0) cout << i << endl;
    }
    fin.close();

    cout << "loading data end" << endl;

    srand((unsigned)time(NULL));
    initSetValue();
    test(port, getCount, setCount, deleteCount);

    return 0;
}
