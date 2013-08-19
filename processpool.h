
#ifndef MM_TRAINING_PROCESS_POOL_H
#define MM_TRAINING_PROCESS_POOL_H

#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
#include "runnable.h"

#define TASKMAXNUM 50

namespace mmtraining {

/**
 * 进程类
 */
class Process : Runnable{
public:
    /**
     * 构造函数
     */
    Process();
    
    /**
     * 构造函数
     */
    Process(Runnable& t);

    /**
     * 析构函数
     */
    virtual ~Process();

    /**
     * 启动进程
     * @return 0: 成功, -1: 失败
     */
    int Start();
    
    /**
     * 获取进程id
     */
    int GetId() const;

    /**
     * 运行进程处理逻辑
     * @return 0: 成功, -1: 失败
     */
    int Run();
    
    /**
     * 终止进程
     */
    int Kill();

    /**
     * 等待进程退出，并回收进程资源
     * @return 0 成功, -1 失败
     */
    int Wait();

protected:

    /**
     * 进程处理逻辑
     */
    virtual int DoRun();
    
    Runnable* target;
    pid_t pid;
};

/**
 * 进程池类
 */
class ProcessPool {
public:
    /**
     * 构造函数
     */
    ProcessPool();
    
    /**
     * 析构函数
     */
    ~ProcessPool();

    /**
     * 启动进程池
     */
    int Start(int procCount, Runnable& target);
    
    /**
     * 终止所有进程
     */
    int KillAll();
    
    /**
     * 等待所有进程退出，并回收进程资源
     * @return 0 成功, -1 失败
     */
    int WaitAll();

protected:
    typedef std::vector<Process*> ProcessVec;
    ProcessVec processes;
};

/**
 * 任务基类
 */
class Task {
public:
    /**
     * 析构函数，释放资源
     */
    virtual ~Task() {}

    /**
     * 任务处理逻辑
     * @return 0 处理成功, -1 处理失败
     */
    virtual int DoTask() = 0;
    
    /**
     * 序列化
     */
    virtual int ToBuffer(void *toBuf) = 0;
    
    /**
     * 反序列化
     */
    virtual int FromBuffer(void *fromBuf) = 0;

    /**
     * get task size
     */
    virtual size_t GetSize() = 0;
};

/**
 * 进程安全的共享内存环形任务队列
 * each queue only handle one type of task
 */
class TaskQueue {
public:
    /**
     * 构造函数
     */
    TaskQueue(Task& taskType);
    
    /**
     * 析构函数
     */
    ~TaskQueue();
    
    /**
     * 添加任务, 并唤醒一个进程处理
     * @return 0: 成功, -1: 失败
     */
    int AddTask(Task& task);
    
    /**
     * 获取任务, 队列空时等待
     * @return 0: 成功, -1: 失败
     */
    Task* GetTask();

    /**
     * @return 0: success, -1 fail
     */
    int GetSize();

protected:
    //semaphore collection, 0 is mutex sem, 1 is queue sem
    int semid;

    void pMutexSem();
    void vMutexSem();
    void pQueueSem();
    void vQueueSem();

    // circle array for tasks
    const int taskMaxNum;
    size_t taskSize;
    // task share memory information
    // struct: queueSize:int + queueIndex:int + task:Task * taskMaxNum
    int shmid;
    void* shmptr;

    int GetQueueSizeInShm();
    int SetQueueSizeInShm(int size);

    int GetQueueIndexInShm();
    int SetQueueIndexInShm(int index);

    /**
     * @Precondition: shmptr is valid
     * @Caution: this method may block callee process until a task has been added
     * @Return: A pointer to Task location in share memory if get success
     *          An Null pointer if error happens
     */
    Task* GetTaskInShm();

    /**
     * @Precondition: shmptr is valid
     * @Caution: this method may wake up a blocked process
     * @Return: 0 if succeed, -1 if failed
     */
    int AddTaskInShm(Task* task);
};

/**
 * 任务处理器
 */
class Processor : public Runnable {
public:
    /**
     * 构造函数
     * @param queue 任务队列
     * @param tt 任务类型
     */
    Processor(TaskQueue& queue, Task& tt);

    /**
     * 析构函数
     */
    ~Processor();

	/**
     * 处理逻辑, 从任务队列中取出工作并处理
     */
    int Run();
    
private:
    
    TaskQueue& taskQueue;
    Task& taskType;
};

/**
 * 任务进程池
 */

class TaskProcessPool {
public:
    /**
     * 构造函数
     */
    TaskProcessPool(Task& taskType);

    /**
     * 析构函数
     */
    ~TaskProcessPool();

    /**
     * 启动进程
     * @param threadCount 启动的进程数
     * @return 0 成功, -1 失败
     */
    int Start(int processCount);

    /**
     * 添加任务，唤醒任务进程处理
     * @param task 任务
     * return 0 成功, -1 失败
     */
    int AddTask(Task& task);

    /**
     * 终止所有进程
     */
    int KillAll();

    /**
     * 等待所有进程退出，并回收进程资源
     * @return 0 成功, -1 失败
     */
    int WaitAll();

private:

    TaskQueue taskQueue;
    Processor processor;
    ProcessPool pool;
};

} // namespace mmtraining

#endif
