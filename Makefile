CC=g++
LIBS=-lpthread
CCO=-c -g

all: server testserver gendatabase getclient setclient deleteclient

clean:
	rm  testserver server gendatabase getclient setclient deleteclient *.o *~ \#*

server: server.o socket.o threadpool.o simplekvsvr.o kvstore.o kvindex.o kvcache.o\
 kvhelper.o kvfile.o memorystore.o kvepoll.o processpool.o
	$(CC) -o server server.o socket.o threadpool.o simplekvsvr.o kvstore.o kvindex.o\
 kvcache.o kvhelper.o kvfile.o memorystore.o kvepoll.o  $(LIBS)

gendatabase: gendatabase.o kvhelper.o
	$(CC) -o gendatabase gendatabase.o kvhelper.o $(LIBS)

testserver: socket.o testserver.o kvhelper.o threadpool.o processpool.o
	$(CC) -o testserver socket.o testserver.o kvhelper.o threadpool.o\
 processpool.o $(LIBS)

getclient: getclient.o socket.o kvhelper.o threadpool.o
	$(CC) -o getclient getclient.o socket.o kvhelper.o threadpool.o $(LIBS)

setclient: setclient.o socket.o kvhelper.o threadpool.o
	$(CC) -o setclient setclient.o socket.o kvhelper.o threadpool.o $(LIBS)

deleteclient: deleteclient.o socket.o kvhelper.o threadpool.o
	$(CC) -o deleteclient deleteclient.o socket.o kvhelper.o threadpool.o $(LIBS)

server.o: server.cpp simplekvsvr.h
	$(CC) $(CCO) server.cpp

setclient.o: setclient.cpp
	$(CC) $(CCO) setclient.cpp

getclient.o: getclient.cpp
	$(CC) $(CCO) getclient.cpp

deleteclient.o: deleteclient.cpp
	$(CC) $(CCO) deleteclient.cpp

testserver.o: testserver.cpp
	$(CC) $(CCO) testserver.cpp

gendatabase.o: gendatabase.cpp
	$(CC) $(CCO) gendatabase.cpp

kvepoll.o: kvepoll.h kvepoll.cpp
	$(CC) $(CCO) kvepoll.cpp

processpool.o: processpool.h processpool.cpp
	$(CC) $(CCO) processpool.cpp

threadpool.o: threadpool.h threadpool.cpp runnable.h
	$(CC) $(CCO) threadpool.cpp

socket.o: socket.h socket.cpp
	$(CC) $(CCO) socket.cpp

simplekvsvr.o: socket.h threadpool.h runnable.h kvstore.h simplekvsvr.h simplekvsvr.cpp
	$(CC) $(CCO) simplekvsvr.cpp

kvstore.o: kvstore.h kvstore.cpp kvindex.h kvfile.h kvstat.h kvcache.h
	$(CC) $(CCO) kvstore.cpp

memorystore.o: memorystore.h memorystore.cpp
	$(CC) $(CCO) memorystore.cpp

kvindex.o: kvindex.h kvindex.cpp kvfile.h kvconfig.h memorystore.h
	$(CC) $(CCO) kvindex.cpp

kvfile.o: kvfile.h kvfile.cpp kvconfig.h kvstat.h kvhelper.h
	$(CC) $(CCO) kvfile.cpp

kvcache.o: kvconfig.h kvcache.h kvcache.cpp
	$(CC) $(CCO) kvcache.cpp

kvhelper.o: kvhelper.h kvhelper.cpp
	$(CC) $(CCO) kvhelper.cpp

