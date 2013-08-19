
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
 * ������
 */
class Process : Runnable{
public:
    /**
     * ���캯��
     */
    Process();
    
    /**
     * ���캯��
     */
    Process(Runnable& t);

    /**
     * ��������
     */
    virtual ~Process();

    /**
     * ��������
     * @return 0: �ɹ�, -1: ʧ��
     */
    int Start();
    
    /**
     * ��ȡ����id
     */
    int GetId() const;

    /**
     * ���н��̴����߼�
     * @return 0: �ɹ�, -1: ʧ��
     */
    int Run();
    
    /**
     * ��ֹ����
     */
    int Kill();

    /**
     * �ȴ������˳��������ս�����Դ
     * @return 0 �ɹ�, -1 ʧ��
     */
    int Wait();

protected:

    /**
     * ���̴����߼�
     */
    virtual int DoRun();
    
    Runnable* target;
    pid_t pid;
};

/**
 * ���̳���
 */
class ProcessPool {
public:
    /**
     * ���캯��
     */
    ProcessPool();
    
    /**
     * ��������
     */
    ~ProcessPool();

    /**
     * �������̳�
     */
    int Start(int procCount, Runnable& target);
    
    /**
     * ��ֹ���н���
     */
    int KillAll();
    
    /**
     * �ȴ����н����˳��������ս�����Դ
     * @return 0 �ɹ�, -1 ʧ��
     */
    int WaitAll();

protected:
    typedef std::vector<Process*> ProcessVec;
    ProcessVec processes;
};

/**
 * �������
 */
class Task {
public:
    /**
     * �����������ͷ���Դ
     */
    virtual ~Task() {}

    /**
     * �������߼�
     * @return 0 ����ɹ�, -1 ����ʧ��
     */
    virtual int DoTask() = 0;
    
    /**
     * ���л�
     */
    virtual int ToBuffer(void *toBuf) = 0;
    
    /**
     * �����л�
     */
    virtual int FromBuffer(void *fromBuf) = 0;

    /**
     * get task size
     */
    virtual size_t GetSize() = 0;
};

/**
 * ���̰�ȫ�Ĺ����ڴ滷���������
 * each queue only handle one type of task
 */
class TaskQueue {
public:
    /**
     * ���캯��
     */
    TaskQueue(Task& taskType);
    
    /**
     * ��������
     */
    ~TaskQueue();
    
    /**
     * �������, ������һ�����̴���
     * @return 0: �ɹ�, -1: ʧ��
     */
    int AddTask(Task& task);
    
    /**
     * ��ȡ����, ���п�ʱ�ȴ�
     * @return 0: �ɹ�, -1: ʧ��
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
 * ��������
 */
class Processor : public Runnable {
public:
    /**
     * ���캯��
     * @param queue �������
     * @param tt ��������
     */
    Processor(TaskQueue& queue, Task& tt);

    /**
     * ��������
     */
    ~Processor();

	/**
     * �����߼�, �����������ȡ������������
     */
    int Run();
    
private:
    
    TaskQueue& taskQueue;
    Task& taskType;
};

/**
 * ������̳�
 */

class TaskProcessPool {
public:
    /**
     * ���캯��
     */
    TaskProcessPool(Task& taskType);

    /**
     * ��������
     */
    ~TaskProcessPool();

    /**
     * ��������
     * @param threadCount �����Ľ�����
     * @return 0 �ɹ�, -1 ʧ��
     */
    int Start(int processCount);

    /**
     * ������񣬻���������̴���
     * @param task ����
     * return 0 �ɹ�, -1 ʧ��
     */
    int AddTask(Task& task);

    /**
     * ��ֹ���н���
     */
    int KillAll();

    /**
     * �ȴ����н����˳��������ս�����Դ
     * @return 0 �ɹ�, -1 ʧ��
     */
    int WaitAll();

private:

    TaskQueue taskQueue;
    Processor processor;
    ProcessPool pool;
};

} // namespace mmtraining

#endif
