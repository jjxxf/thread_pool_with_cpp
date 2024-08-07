#pragma once

#include <iostream>
#include <queue>
#include <functional>
#include <memory>
#include <functional>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <semaphore>
#include <map>

class ThreadPool
{
public:
    ThreadPool(int min = 2, int max = std::thread::hardware_concurrency());// 线程数量最大值默认为cpu核心数
    ~ThreadPool();

    int getBusyThreadNum();  // 获取忙线程数
    int getLiveThreadNum(); // 获取存活线程数
    int getExitThreadNum(); // 获取要退出的线程数
    bool getPoolExitFlag(); // 获取线程池退出标志

    void addTask(std::function<void()> task); // 添加任务


private:
    static void worker_func(void* arg); // 工作线程函数

    std::queue<std::function<void()>> task_queue_; // 任务队列
    std::map<std::thread::id, std::thread> thread_map_; // 线程id和线程的映射, 为了在销毁线程后能够析构线程实例

    std::vector<std::thread::id> exited_threads_; // 已退出的线程
    std::thread manager_thread_; // 管理线程

    int getTaskNum(); // 获取任务数

    int max_thread_num_; // 最大线程数
    int min_thread_num_; // 最小线程数

    std::atomic<int> live_thread_num_; // 存活线程数
    std::atomic<int> busy_thread_num_; // 忙线程数
    std::atomic<int> exit_thread_num_; // 要退出的线程数

    std::atomic<bool> exit_pool_flag_; // 退出标志，true退出

    std::condition_variable cv_; // 条件变量， 任务队列空时等待， 添加任务后唤醒
    std::mutex task_queue__mtx_; // 互斥锁
    std::mutex exited_threads_mtx_; // 互斥锁


};