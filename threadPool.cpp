#pragma once
#include "threadPool.h"



ThreadPool::ThreadPool(int min, int max)
:min_thread_num_(min), max_thread_num_(max), exit_pool_flag_(false), 
live_thread_num_(0), busy_thread_num_(0), exit_thread_num_(0){

    task_queue_ = new TaskQ;    

    // 首先创建最小数量的线程
    for(int i = 0; i < min; i++){
        // std::thread t(worker_func, this);
        // thread_queue_.push(std::move(t)); // 先创建再存入队列需要使用std::move
        thread_queue_.push(std::thread(worker_func, this)); // 直接存入队列
    }

    std::cout << "creat min number thread -------" << std::endl;
    
    // 用lambda表达式创建管理者线程
    manager_thread_ = std::thread([this](){
        const int NUMBER = 2; // 每次增加或减少的线程数量
        std::cout << "creat manager thread_ -------" << std::endl;
        while (this->exit_pool_flag_ == false)
        {   
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            int task_number = this->task_queue_->getQueueSize();
            std::cout << " -------task_number: " << task_number << std::endl;

            {
                std::lock_guard<std::mutex> mtx_guard(mtx_);
                // 如果线程数即将小于任务队列中的任务数，且线程数小于最大线程数减2
                if(this->live_thread_num_ <  task_number + 2 
                && this->live_thread_num_ <= max_thread_num_ - 2){
                    for(int i = 0; i< NUMBER; i++){
                        thread_queue_.push(std::thread(worker_func, this)); // 创建线程并存入队列
                        this->live_thread_num_++; // 存活的线程数加1
                        std::cout << "creat one worker thread -------" << std::endl;
                    }
                }

                // 如果线程数大于最小线程数加2倍的忙线程数，且线程数大于最小线程数加2
                if(this->live_thread_num_ > 2 * this->busy_thread_num_ 
                && this->live_thread_num_ >= min_thread_num_ + 2){
                    this->exit_thread_num_ = NUMBER;
                    for(int i = 0; i< NUMBER; i++){
                        this->cv_.notify_one(); // 唤醒线程，使其退出
                    }
                }
            }

        }
    });    
}


ThreadPool::~ThreadPool(){
    std::cout << "destroy Thread Pool" << std::endl;
    exit_pool_flag_ = true;
    manager_thread_.join(); // 等待管理者线程结束
    std::cout << "Thread Queue size: "  << thread_queue_.size() << std::endl;
    std::cout << "Alive Thread number: "  << getLiveThreadNum() << std::endl;
    std::cout << "Busy Thread number: "  << getBusyThreadNum() << std::endl;

    cv_.notify_all();
    int i = 0;
    // 等待所有线程结束
    while(!thread_queue_.empty()){
        thread_queue_.front().join();
        thread_queue_.pop();
        std::cout << "join thread: "  << ++i << std::endl;
    }
    if(task_queue_ != nullptr){
        delete task_queue_;
        task_queue_ = nullptr;
    }
}

void* ThreadPool::worker_func(void* arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    std::cout << std::this_thread::get_id() << "  create !! " << std::endl;
    while(true){
        // 如果任务队列为空，且线程池没有退出标志，则等待
        while(pool->task_queue_->getQueueSize() == 0 
        && !pool->exit_pool_flag_){
            std::cout << std::this_thread::get_id() << "  wating !! " << std::endl;
            
            {
                std::unique_lock<std::mutex> mtx_guard(pool->mtx_);
                pool->cv_.wait(mtx_guard); // 等待唤醒

                // 即使不满足下面的退出条件也要减1，使exit_thread_num_归0，防止后面使其他线程退出
                if(pool->exit_thread_num_ > 0){
                    pool->exit_thread_num_--; 

                    if(pool->live_thread_num_ > pool->min_thread_num_){
                        pool->live_thread_num_--;
                        std::cout << "destroy one worker thread -------" << std::endl;
                        return nullptr; // 退出线程
                    }
                }
            }

        }
        if(pool->exit_pool_flag_){
            return nullptr; // 退出标志为true, 退出线程
        }

        if(pool->task_queue_->getQueueSize() != 0){
            Task task;
            if(pool->task_queue_->getTask(task)){ // 获取任务
                pool->busy_thread_num_++; // 忙线程数加1
                task.func(task.arg); // 执行任务
                delete task.arg; // 释放任务参数
		        task.arg = nullptr;
                pool->busy_thread_num_--; // 忙线程数减1
            }else{
                std::cout << "task queque is empty, get null task" << std::endl;
            }   
        }
    }
}

int ThreadPool::getBusyThreadNum(){
    return busy_thread_num_;
}
int ThreadPool::getLiveThreadNum(){
    return live_thread_num_;
}
int ThreadPool::getExitThreadNum(){
    return exit_thread_num_;
}
bool ThreadPool::getPoolExitFlag(){
    return exit_pool_flag_;
}

void ThreadPool::addTask(Task& task){
    
    if(exit_pool_flag_)
        return;

    task_queue_->addTask(task);
    cv_.notify_one();  // 添加了任务，唤醒线程使其执行任务
    // std::cout << "Add Task" << std::endl;

}


