
#include "kvfile.h"
#include "kvconfig.h"
#include "kvhelper.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

extern int errno;

using std::string;
using std::cout;
using std::endl;

namespace mmtraining {

KVFile::KVFile(const char *filename) {
    pFile = fopen(filename, "a+");
    strcpy(this->filename, filename);
    writeOff = 0;

    if (pFile == NULL) {
	cout << "KVFile::KVFile open file error: " << strerror(errno) << endl;
	cout << "Exit" << endl;
	exit(1);
    }

    int err = pthread_rwlock_init(&lock, NULL);
    if (err != 0) {
	cout << "KVFile::KVFile init rwlock error: " << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }

    if (fseek(pFile, 0, SEEK_END) != 0 || (writeOff = ftell(pFile)) == -1L) {
	cout << "KVFile::KVFile get writeOff error: " << strerror(err) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}

KVFile::~KVFile() {
    if (pFile != NULL && fclose(pFile) == EOF) {
	cout << "KVFile::KVFile close file error: " << strerror(errno) << endl;
	cout << "Exit" << endl;
	exit(1);
    }
}


OFFSETTYPE KVFile::WritePairAppend(string &key, string &value) {
    OFFSETTYPE ret = 0;

    GetWriteLock(&lock);

    string temp = key + " " + value + "\n";
    if (fwrite(temp.c_str(), temp.size(), 1, pFile) != 1) {
	cout << "KVFile::WritePairAppend error: write string error: " 
	     << strerror(errno) << endl;
	ret = -1;
	goto end;
    } 
    ret = writeOff;
    writeOff += temp.size();

 end:
    ReleaseWriteLock(&lock);
    return ret;
}

int KVFile::ReadSeek(OFFSETTYPE off) {
    if (off < 0) {
	cout << "KVFile::ReadSeek error: off < 0" << endl;
	return -1;
    } 
    
    int ret = 0;
    GetReadLock(&lock);
    if (fseek(pFile, off, SEEK_SET) != 0) {
	cout << "KVFile::ReadSeek error: lseek fail" << endl;
	ret = -2;
    }
    ReleaseReadLock(&lock);

    return 0;
}

int KVFile::ReadPair(string &key, string &value, OFFSETTYPE off) {
    char *buf = new char[KVDATABUFSIZE];
    int fd = fileno(pFile);

    int ret = 0;

    GetReadLock(&lock);
    if (pread(fd, buf, KVDATABUFSIZE, off) == -1) {
	cout << "KVFile::ReadPair read error: " << strerror(errno) << endl;
	ret = -1;
    } else if (strcmp(buf, "") != 0) {
	string s(buf);
	std::stringstream ss(s, std::ios_base::in);
	ss >> key >> value;
    } else {
	ret = -2;
    }
    ReleaseReadLock(&lock);

    delete []buf;

    return ret;
}

int KVFile::ReadPairAhead(string &key, string &value) {
    char *buf = new char[KVDATABUFSIZE];
    int ret = 0;

    GetReadLock(&lock);
    if (fgets(buf, KVDATABUFSIZE, pFile) == NULL && ferror(pFile)) {
	cout << "KVFile::ReadPairAhead read error: " << strerror(errno) << endl;
	ret = -1;
    } else if (strcmp(buf, "") != 0) {
	string s(buf);
	std::stringstream ss(s, std::ios_base::in);
	ss >> key >> value;
    } else {
	ret = -2;
    }
    ReleaseReadLock(&lock);
    
    delete []buf;
    return ret;
}


bool KVFile::Eof() {
    return feof(pFile);
}


OFFSETTYPE KVFile::GetOff() {
    return ftell(pFile);
}

void KVFile::GetStat(kvstat& stat) {
    GetReadLock(&lock);
    stat.fileSize = writeOff;
    ReleaseReadLock(&lock);
}


int KVFile::Rename(const char *str) {
    int ret = 0;
    GetWriteLock(&lock);
    if (fclose(pFile) == EOF) {
	cout << "KVFile::Rename close old file " << filename << " error: "
	     << strerror(errno) << endl;
	ret = -1;
	goto end;
    } else if (rename(filename, str) != 0) {
	cout << "KVFile::Rename rename old file " << filename << " to new file "
	     << str << " error: " << strerror(errno) << endl;

	pFile = fopen(filename, "a+");
	if (pFile == NULL) {
	    cout << "KVFile::Rename open old file " << filename << " error: "
		 << strerror(errno) << endl;
	    cout << "process exit" << endl;
	    exit(1);
	}
	    
	ret = -2;
	goto end;
    } else {
	strcpy(filename, str);
    }

    pFile = fopen(filename, "a+");
    if (pFile == NULL) {
	cout << "KVFile::Rename open new file " << filename << " error: "
	     << strerror(errno) << endl;
	cout << "process eixt" << endl;
	exit(1);
    }

 end:
    ReleaseWriteLock(&lock);
    return ret;
}

}
