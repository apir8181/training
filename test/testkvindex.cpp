
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <assert.h>
#include "kvconfig.h"
#include "kvindex.h"
#include "kvfile.h"
#include "kvhelper.h"

using namespace mmtraining;
using namespace std;

void genFile() {
    srand((unsigned)time(NULL));

    cout << "Generating " << FILENAME << " start" << endl;

    fstream fs;
    fs.open(FILENAME, ios_base::out);
    
    string key = "", value = "";
    int vlen;
    for (int i = 0; i < NTESTDATA; i ++) {
	vlen = rand() % (VALUEMAXSIZE - 5) + 5;
	key.clear();
	value.clear();
	genRandomStr(key, KEYNAXSIZE);
	genRandomStr(value, vlen);
	
	if (i % 10000 == 0) {
	    cout << "\t" << i << endl;
	}
	fs << key << ' ' << value << endl;
    }

    fs.close();

    cout << "Generating " << FILENAME << " end" << endl;
}

void genTestFile() {
    cout << "Generating " << TESTFILENAME << " start" << endl;

    ifstream fin;
    fin.open(FILENAME);
    
    ofstream fout;
    fout.open(TESTFILENAME);
    
    string key, value;
    for (int i = 0; i < NTESTDATA; ++ i) {
	fin >> key >> value;
	fout << key << endl;
	if (fin.eof()) break;
    }

    fin.close();
    fout.close();

    cout << "Generating " << TESTFILENAME << " end" << endl;
}

int main(int argc, char **argv) {
    genFile();
    genTestFile();
    return 0;
}
