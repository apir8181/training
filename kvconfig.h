
#ifndef MMTRAINING_KVCONFIG_H
#define MMTRAINING_KVCONFIG_H

//kvcache
#define NBUCKETSIZE 10
#define NCACHEBUCKET 10000
#define MAXKEYSIZE 8
#define MAXVALUESIZE 1024

//kvindex
#define OFFSETTYPE long long
#define NINDEXBUCKET 10000

//kvfile
#define FILENAME "kvdatabase"
#define NEWFILENAME "kvdatabasenew"
#define KEYMAXSIZE 8
#define VALUEMAXSIZE 1024
#define KVDATABUFSIZE (KEYMAXSIZE + VALUEMAXSIZE + 32)

//memorystore

//kvstore
#define ISREFACTOR -255

//kvepoll
#define NEPOLLBUCKET 65536
#define EPOLLSIZE 100
#define EVENTARRAYSIZE 200
#define SECONDPERPLUSE 100
#define TIMEOUTSECOND 60

//test
#define SERVERIP "127.0.0.1"
#define TESTFILENAME "kvdatabasetest"
#define NTESTDATA 1000000
#define PERITERTIME 5
#define TESTTIME 60

#endif
