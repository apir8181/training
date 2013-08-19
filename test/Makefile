CC=g++
LIBS=-lpthread
CCO=-c -g

all: server testserver gendatabase

clean:
	rm  testserver server gendatabase *.o *~ \#*

server: server.o socket.o threadpool.o simplekvsvr.o kvstore.o kvindex.o kvcache.o\
 kvhelper.o kvfile.o memorystore.o kvepoll.o processpool.o
	$(CC) -o server server.o socket.o threadpool.o simplekvsvr.o kvstore.o kvindex.o\
 kvcache.o kvhelper.o kvfile.o memorystore.o kvepoll.o  $(LIBS)

gendatabase: gendatabase.o kvhelper.o
	$(CC) -o gendatabase gendatabase.o kvhelper.o $(LIBS)

testserver: socket.o testserver.o kvhelper.o threadpool.o processpool.o
	$(CC) -o testserver socket.o testserver.o kvhelper.o threadpool.o processpool.o $(LIBS)

# testkvstore: testkvstore.o kvstore.o kvindex.o kvcache.o kvhelper.o kvfile.o memorystore.o
# 	$(CC) -o testkvstore kvstore.o kvindex.o kvcache.o kvhelper.o kvfile.o memorystore.o testkvstore.o $(LIBS)

# testkvindex: testkvindex.o kvindex.o kvfile.o kvhelper.o memorystore.o
# 	$(CC) -o testkvindex testkvindex.o kvindex.o kvfile.o kvhelper.o memorystore.o $(LIBS)

# testkvfile: testkvfile.o kvfile.o kvhelper.o
# 	$(CC) -o testkvfile testkvfile.o kvfile.o kvhelper.o $(LIBS)

# testkvcache: testkvcache.o kvcache.o kvhelper.o
# 	$(CC) -o testkvcache testkvcache.o kvcache.o kvhelper.o $(LIBS)


server.o: server.cpp simplekvsvr.h
	$(CC) $(CCO) server.cpp

testserver.o: testserver.cpp
	$(CC) $(CCO) testserver.cpp

gendatabase.o: gendatabase.cpp
	$(CC) $(CCO) gendatabase.cpp

# testkvstore.o: testkvstore.cpp kvstore.h kvindex.h kvcache.h kvstat.h
# 	$(CC) $(CCO) testkvstore.cpp

# testkvindex.o: testkvindex.cpp kvindex.h
# 	$(CC) $(CCO) testkvindex.cpp

# testkvfile.o: testkvfile.cpp kvfile.h kvconfig.h
# 	$(CC) $(CCO) testkvfile.cpp

# testkvcache.o: testkvcache.cpp
# 	$(CC) $(CCO) testkvcache.cpp


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

