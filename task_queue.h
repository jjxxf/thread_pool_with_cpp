#pragma once

#include <iostream>
#include <queue>
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>

struct Task{
    std::function<void*(void*)> func;
    void* arg;
    Task():func(nullptr), arg(nullptr){};
    Task(std::function<void*(void*)> f, void* a): func(f), arg(a){}
};

class TaskQ
{

public:

    TaskQ();

    ~TaskQ();

    int getQueueSize(); // 获取任务队列的大小

    bool addTask(Task &t); // 添加任务

    bool getTask(Task &t); // 获取任务,并将任务弹出


private:
    std::mutex mtx_; // 互斥锁

    std::queue<Task> task_queue_; // 任务队列
    
};