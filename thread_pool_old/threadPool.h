#pragma once

#include <iostream>
#include <queue>
#include <functional>
#include <memory>
#include "task_queue.h"
#include <thread>
#include <condition_variable>
#include <atomic>
#include <semaphore>

class ThreadPool
{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

    int getBusyThreadNum();  // 获取忙线程数
    int getLiveThreadNum(); // 获取存活线程数
    int getExitThreadNum(); // 获取要退出的线程数
    bool getPoolExitFlag(); // 获取线程池退出标志

    void addTask(Task &task); // 添加任务


private:
    static void* worker_func(void* arg); // 工作线程函数

    TaskQ* task_queue_; // 任务队列
    std::queue<std::thread> thread_queue_; // 线程队列
    std::thread manager_thread_; // 管理线程

    int max_thread_num_; // 最大线程数
    int min_thread_num_; // 最小线程数

    std::atomic<int> live_thread_num_; // 存活线程数
    std::atomic<int> busy_thread_num_; // 忙线程数
    std::atomic<int> exit_thread_num_; // 要退出的线程数

    std::atomic<bool> exit_pool_flag_; // 退出标志，true退出

    std::condition_variable cv_; // 条件变量， 任务队列空时等待， 添加任务后唤醒
    std::mutex mtx_; // 互斥锁

};