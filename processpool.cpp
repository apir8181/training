
#include "processpool.h"
#include <signal.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

using std::cout;
using std::endl;

extern int errno;

void echo_time(const char* str) {
    // time_t     now = time(0);
    // struct tm  tstruct;
    // char       buf[80];
    // tstruct = *localtime(&now);
    // strftime(buf, sizeof(buf), "%X", &tstruct);
    // cout << getpid() << " " << str << ", time " << buf << endl;

    timeval t;
    gettimeofday(&t, NULL);
    cout << getpid() << " " << str << " " << t.tv_sec << "." << t.tv_usec << endl;
}

namespace mmtraining {

/////////////////////////////////////////////////Process

Process::Process() : target(0) {}
    
Process::Process(Runnable& t) : target(&t) {}

Process::~Process() {
    // TODO: 释放资源
}

int Process::Start() {
    // TODO: 完成代码

    if ((pid = fork()) < 0) {
	cout << getpid() << " process fork child error" << endl;
	return -1;
    } else if (pid == 0) { //child process
	int child_ret = Run();
	exit(child_ret); //run success exit(0), else exit(1)
    }
    //parent process

    return 0;
}

/**
 * @Precondition: Callee should call process start once
 * otherwise it will return an unknown process id
 */
int Process::GetId() const {
    // TODO: 完成代码
    return pid;
}

int Process::Run() {
    if (target != NULL) {
        return target->Run();
    } else {
        return DoRun();
    }
}

/**
 * @Precondition: Callee should call process start once
 */
int Process::Kill() {
    // TODO: 完成代码
    int ret = 0;
    cout << getpid() << " kill process " << pid << endl;

    if (kill(pid, SIGTERM) == -1) {
	cout << "kill " << pid << " failed" << endl;
	ret = -1;
    } else if (waitpid(pid, NULL, 0) != pid) {
	cout << "wait " << pid << " error" << endl;
	ret = -1;
    }

    return ret;
}

int Process::Wait() {
    // TODO: 完成代码
    int ret = 0;
    if (waitpid(pid, NULL, 0) != pid) {
	cout << "wait " << pid << " error" << endl;
	ret = -1;
    }
    return ret;
}

int Process::DoRun() {
    return 0;
}

/////////////////////////////////////////////////ProcessPool

ProcessPool::ProcessPool() {
    processes.clear();
}
    
ProcessPool::~ProcessPool() {
    // TODO: 释放资源
    //KillAll();
}

/**
 * If unable to start procCount process, it will try to start as many processes
 * as possible
 */
int ProcessPool::Start(int procCount, Runnable& target) {
    // TODO: 完成代码
    for (int i = 0; i < procCount; i ++) {
	Process* process = new Process(target);
	bool success = true;

	if (process == NULL) {
	    cout << "process pool start failed in " << i << "th process .\n" 
		 << "\t because of not enough memory" << endl;
	    success = false;
	} else if (process->Start() == -1) {
	    cout << "process poll start filed in " << i << "th process .\n"
		 << "\t because of starting failed" << endl;
	    success = false;
	}

	if (!success)
	    return -1;

	processes.push_back(process);
    }

    return 0;
}

int ProcessPool::KillAll() {
    // TODO: 完成代码
    ProcessVec::iterator it = processes.end();
    while (it != processes.begin()) {
	it --;
	Process* process = *it;
	if (process->Kill() != 0) {
	    cout << "ProcessPool kill all failed in killing process "
		 << process->GetId() << endl;
	    cout << errno << endl;
	    return -1;
	}
	delete process;

	it = processes.erase(it);
    }

    return 0;
}
    
int ProcessPool::WaitAll() {
    // TODO: 完成代码
    int ret = 0;
    ProcessVec::iterator it = processes.end();
    while (it != processes.begin()) {
	it --;
	Process* process = *it;
	if (process->Kill() != 0) {
	    cout << "ProcessPool wait all failed in waitting process "
		 << process->GetId() << endl;
	    ret = -1;
	    break;
	}
	delete process;
	it = processes.erase(it);
    }

    return ret;
}

/////////////////////////////////////////////////TaskQueue

TaskQueue::TaskQueue(Task& taskType) : taskMaxNum(TASKMAXNUM) {
    taskSize = taskType.GetSize();

    //create share memory start
    size_t info_size = sizeof(int) * 2;
    size_t total_size = info_size + taskSize * taskMaxNum;
    shmid = shmget(IPC_PRIVATE, total_size, 0600 | IPC_CREAT | IPC_EXCL); 
    if (shmid < 0) {
	cout << "task queue share memory create id failed" << endl;
	exit(1);
    } else if ((shmptr = shmat(shmid, 0, 0)) == (void *)-1) {
	cout << "task queue init share memory error in linking to share memory" 
	     << endl;
	exit(1);
    } else {
	memset(shmptr, 0, info_size);
	if (shmdt(shmptr) != 0) {
	    cout << "task queue init share memory err in deleting the link " 
		 << "to share memory" << endl;
	    exit(1);
	}
    }
    shmptr = (void *)-1;
    //create share memory end

    //semaphore init start
    if ((semid = semget(IPC_PRIVATE, 2, 0600 | IPC_CREAT | IPC_EXCL)) == -1) {
	cout << "task queue semaphore semget error" << endl;
	exit(1);
    }
    unsigned short semval[2] = {1, 0};
    if (semctl(semid, 0, SETALL, semval) == -1) {
	cout << "task queue semaphore val init error" << endl;
	exit(1);
    }

}
    
TaskQueue::~TaskQueue() {
    if (shmptr != (void *)-1)
	if (shmctl(shmid, IPC_RMID, NULL) != 0) {
	    cout << getpid() << " process failed to delete the share memory link "
		 << endl;
	    exit(1);
	}
    // how to delete semephora?
}
    
int TaskQueue::AddTask(Task& task) {
    // TODO: 完成代码
    // link share memory to local process virtual memory
    if (shmptr == (void *)-1) {
	shmptr = shmat(shmid, 0, 0);
	if (shmptr == (void *)-1) {
	    cout << "share memory add task error" << endl;
	    return -1;
	}
    } else if (task.GetSize() != taskSize) {
	cout << "TaskQueue add a task of invalid type" << endl;
	return -1;
    }

    return AddTaskInShm(&task);
}
    
Task* TaskQueue::GetTask() {
    // TODO: 完成代码
    if (shmptr == (void*)-1) {
	shmptr = shmat(shmid, 0, 0);
	if (shmptr == (void *)-1) {
	    cout << "task queue get task error in linking to shared memory" 
		 << endl;
	    return NULL;
	}
    } 

    return GetTaskInShm();
}

int TaskQueue::GetSize() {
    pMutexSem();
    int ret = GetQueueSizeInShm();
    vMutexSem();
    return ret;
}

void TaskQueue::pMutexSem() {
    echo_time("p mutex start");
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    while (semop(semid, &buf, 1) == -1) {
	if (errno != EAGAIN) {
	    cout << getpid() << " process p operation error with errno " 
		 << errno << endl;
	    exit(1);
	} 
    }
}

void TaskQueue::vMutexSem() {
    echo_time("v mutex start");
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    while (semop(semid, &buf, 1) == -1) {
	if (errno != EAGAIN) {
	    cout << getpid() << " process p operation error with errno " 
		 << errno << endl;
	    exit(1);
	} 
    }
}

void TaskQueue::pQueueSem() {
    echo_time("p queue start");
    struct sembuf buf;
    buf.sem_num = 1;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    while (semop(semid, &buf, 1) == -1) {
	if (errno != EAGAIN) {
	    cout << getpid() << " process p operation error with errno " 
		 << errno << endl;
	    exit(1);
	} 
    }
}

void TaskQueue::vQueueSem() {
    echo_time("v queue start");
    struct sembuf buf;
    buf.sem_num = 1;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    while (semop(semid, &buf, 1) == -1) {
	if (errno != EAGAIN) {
	    cout << getpid() << " process p operation error with errno " 
		 << errno << endl;
	    exit(1);
	} 
    }
}

int TaskQueue::GetQueueSizeInShm() {
    if (shmptr == (void*)-1)
	return -1;
    int *ptr = (int*)shmptr;
    return *ptr;
}

int TaskQueue::SetQueueSizeInShm(int size) {
    if (shmptr == (void*)-1)
	return -1;
    memcpy(shmptr, &size, sizeof(size));
    return 0;
}

int TaskQueue::GetQueueIndexInShm() {
    if (shmptr == (void*)-1)
	return -1;
    int *ptr = (int*)((char*)shmptr + sizeof(int));
    return *ptr;
}

int TaskQueue::SetQueueIndexInShm(int index) {
    if (shmptr == (void*)-1)
	return -1;
    int *ptr = (int*)((char*)shmptr + sizeof(int));
    memcpy(ptr, &index, sizeof(int));
    return 0;
}

Task* TaskQueue::GetTaskInShm() {
    Task* retPtr = NULL;

    pQueueSem();
    pMutexSem();
    int queueSize = GetQueueSizeInShm(), queueIndex = GetQueueIndexInShm();
    if (queueSize == -1 || queueIndex == -1) {
	cout << "process " << getpid() << " get queue task info error "
	     << "in get task" << endl;
    } else{
	retPtr = (Task *)((char*)shmptr + sizeof(int) * 2 + 
			  queueIndex * taskSize);
	queueSize -= 1;
	queueIndex = (queueIndex + 1) % taskMaxNum;
	if (SetQueueSizeInShm(queueSize) == -1 || 
	    SetQueueIndexInShm(queueIndex) == -1) {
	    cout << "process " << getpid() << "set queue task info error" 
		 << endl;
	    retPtr = NULL;
	}
	
	char str[50];
	sprintf(str, "get! queuesize:%d queueindex:%d", queueSize, queueIndex);
	echo_time(str);
    }
    vMutexSem();

    return retPtr;
}

int TaskQueue::AddTaskInShm(Task* task) {
    //lock
    pMutexSem();

    int queueSize = GetQueueSizeInShm(), queueIndex = GetQueueIndexInShm();
    int ret = 0;
    if (queueSize == -1 || queueIndex == -1) {
	cout << "process " << getpid() << " add task into task queue error "
	     << "in getting queue info" << endl;
	ret = -1;
    } else if (queueSize == taskMaxNum) {
	cout << "process " << getpid() << " add task into task queue error "
	     << " because taskqueue is full now" << endl;
	ret = -1;
    } else {
	int index = (queueIndex + queueSize) % taskMaxNum;
	void* ptr = (char*)shmptr + sizeof(int) * 2 + index * taskSize;
	memcpy(ptr, task, taskSize);
	queueSize += 1;
	if (SetQueueSizeInShm(queueSize) == -1) {
	    cout << "process " << getpid() << " set queue task info error in "
		 << "add task" << endl;
	    ret = -1;
	} else {
	    char str[50];
	    sprintf(str, "add queuesize:%d queueindex:%d", queueSize, queueIndex);
	    echo_time(str);
	    vQueueSem();
	}
    }

    vMutexSem();
    return ret;
}

/////////////////////////////////////////////////Processor

Processor::Processor(TaskQueue& queue, Task& tt) : taskQueue(queue), taskType(tt) {}

Processor::~Processor() {
    // TODO: 释放资源
    // nothing to do?
}

int Processor::Run() {
    // TODO: 完成代码
    while (1) {
	Task* task = taskQueue.GetTask();
	if (task != NULL)
	    task->DoTask();
    }

    return 0;
}

/////////////////////////////////////////////////TaskProcessPool

TaskProcessPool::TaskProcessPool(Task& dummy) : 
    taskQueue(dummy), processor(taskQueue, dummy) {}

TaskProcessPool::~TaskProcessPool() {
    // TODO: 释放资源
    // nothing to be done
}

int TaskProcessPool::Start(int processCount) {
    return pool.Start(processCount, processor);
}

int TaskProcessPool::AddTask(Task& task) {
    return taskQueue.AddTask(task);
}

int TaskProcessPool::KillAll() {
    return pool.KillAll();
}

int TaskProcessPool::WaitAll() {
    return pool.WaitAll();
}

} // namespace mmtraining
