import telnetlib, socket
import sys, os, random, string
from time import sleep
from multiprocessing import Process, Value
from datetime import datetime
import thread, threading

keyCount = 100000
strs = []
lock = threading.Lock()
num = 0

def get_run(port):
    t = telnetlib.Telnet()
    t.open("localhost", port)

    global num
    count = 0
    while True:
        try:
            s = random.choice(strs)
            t.write("get " + s)
        except socket.error:
            print str(os.getpid()) + " get process write error: socket close"
            break
            
        try:
            s = t.read_until("\n", 1)
            if len(s) == 0 or s[len(s) - 1] != '\n':
                continue

        except EOFError:
            print str(os.getpid()) + " get process read error: socket close"
            break
        
        count += 1
        if count == 100:
            lock.acquire()
            num += 100
            count = 0
            lock.release()

    lock.acquire()
    num += count
    lock.release()
    t.close()


def set_run(port):
    t = telnetlib.Telnet()
    t.open("localhost", port)

    global num
    count = 0
    while True:
        try:
            key = random.choice(strs)
            value = "".join([random.choice(string.lowercase) for i in xrange(100)])
            t.write("set " + key + " " + value)
        except socket.error:
            print str(os.getpid()) + " set process read error: socket close"
            break
        
        try:
            s = t.read_until("\n", 1)
            if len(s) == 0 or s[len(s) - 1] != '\n':
                continue

        except EOFError:
            print str(os.getpid()) + " set process write error: socket close"
            break

        count += 1
        if count == 100:
            lock.acquire()
            num.value += 100
            count = 0
            lock.release()

    lock.acquire()
    num += count
    lock.release()
    t.close()


def delete_run(port):
    t = telnetlib.Telnet()
    t.open("localhost", port)

    global num
    count = 0
    while True:
        try:
            key = random.choice(strs)
            t.write("delete " + key)
        except socket.error:
            print str(os.getpid()) + " delete process read error: socket close"
            break

        try:
            s = t.read_until("\n", 1)
            if len(s) == 0 or s[len(s) - 1] != '\n':
                continue

        except EOFError:
            print str(os.getpid()) + " delete process write error: socket close"
            break

        count += 1
        if count == 100:
            lock.acquire()
            num.value += 100
            count = 0
            lock.release()

    lock.acquire()
    num += count
    t.close()
    lock.release()
    

def create_processes(processCount, port, func):
    processes = []
    for i in xrange(processCount):
        p = threading.Thread(target=func, args=(port, ))
        #p = Process(target=func, args=(num, port))
        processes.append(p)
    
    for i in xrange(processCount):
        processes[i].start()

    return processes


def update_processes(processes, processCount, port, func):
    tempProcesses = []

    for process in processes:
        if process.is_alive():
            tempProcesses.append(process)

    for i in xrange(processCount - len(tempProcesses)):
        p = threading.Thread(target=func, args=(port, ))
        #p = Process(target=func, args=(num, port))
        tempProcesses.append(p)
        p.start()

    processes = tempProcesses


if __name__ == "__main__":

    if len(sys.argv) < 5:
        print "program usage :" + sys.argv[0] + " port getCount setCount deleteCount"
        sys.exit(1)

    f = open("testserverdat", "r")
    for i in xrange(keyCount):
        s = f.readline()
        s = s.rstrip()
        strs.append(s)
    f.close()

    port = sys.argv[1]
    getCount = int(sys.argv[2])
    setCount = int(sys.argv[3])
    deleteCount = int(sys.argv[4])

    beforeTime = datetime.now()

    getProcesses = create_processes(getCount, port, get_run)
    setProcesses = create_processes(setCount, port, set_run)
    deleteProcesses = create_processes(deleteCount, port, delete_run)

    while True:
        update_processes(getProcesses, getCount, port, get_run)
        update_processes(setProcesses, setCount, port, set_run)
        update_processes(deleteProcesses, deleteCount, port, delete_run)

        nowTime = datetime.now()
        lock.acquire()
        print str(nowTime - beforeTime) + " Finish Count:" + str(num)
        lock.release()
