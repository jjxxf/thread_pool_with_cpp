#include "threadPool.h"

ThreadPool::ThreadPool(int min, int max)
:min_thread_num_(min), max_thread_num_(max), exit_pool_flag_(false), 
live_thread_num_(0), busy_thread_num_(0), exit_thread_num_(0){

    // 首先创建最小数量的线程
    for(int i = 0; i < min; i++){
        std::thread t(&worker_func, this);
        thread_map_.emplace(std::make_pair(t.get_id(), std::move(t))); // emplace避免拷贝，提高效率
        live_thread_num_++; // 存活的线程数加1
    }

    std::cout << "creat min number thread -------" << std::endl;
    
    // 用lambda表达式创建管理者线程
    manager_thread_ = std::thread([this](){
        const int NUMBER = 2; // 每次增加或减少的线程数量
        std::cout << "creat manager thread_ -------, id: " << std::this_thread::get_id() << std::endl;
        while (this->exit_pool_flag_ == false)
        {   std::cout << " manager thread run " << std::endl;  
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            int task_number = this->getTaskNum(); // 获取任务数
            std::cout << " -------task_number: " << task_number << std::endl;
            // 如果线程数即将小于任务队列中的任务数，且线程数小于最大线程数减2, 创建线程
            if(this->live_thread_num_ <  task_number + 2 
            && this->live_thread_num_ <= max_thread_num_ - 2){
                for(int i = 0; i< NUMBER; i++){
                    std::thread t(&worker_func, this);
                    thread_map_.emplace(std::make_pair(t.get_id(), std::move(t))); // emplace避免拷贝，提高效率// 创建线程并存入队列
                    this->live_thread_num_++; // 存活的线程数加1
                    std::cout << "creat one worker thread -------" << std::endl;
                }
            }

            // 如果线程数大于最小线程数加2倍的忙线程数，且线程数大于最小线程数加2，销毁线程
            if(this->live_thread_num_ > 2 * this->busy_thread_num_ 
            && this->live_thread_num_ >= min_thread_num_ + 2){
                this->exit_thread_num_ = NUMBER;
                for(int i = 0; i< NUMBER; i++){
                    this->cv_.notify_one(); // 唤醒线程，使其退出
                }

                {
                    std::lock_guard<std::mutex> mtx_guard(this->exited_threads_mtx_);
                    for(const auto &id : this->exited_threads_){
                        auto it = thread_map_.find(id);
                        if(it != thread_map_.end()){
                            if(it->second.joinable()){
                                it->second.join(); // 等待线程结束
                                thread_map_.erase(it); // 从map中删除
                                std::cout << "destroy one worker thread -------" << std::endl;
                            } 
                        }
                    }
                    this->exited_threads_.clear(); // 清空已退出线程
                }
            }        
        }
    });    
}


ThreadPool::~ThreadPool(){
    std::cout << "destroy Thread Pool" << std::endl;
    exit_pool_flag_ = true;
      
    std::cout << "Thread Queue size: "  << thread_map_.size() << std::endl;
    std::cout << "Alive Thread number: "  << getLiveThreadNum() << std::endl;
    std::cout << "Busy Thread number: "  << getBusyThreadNum() << std::endl;

    cv_.notify_all();
    int i = 0;
    // 等待所有线程结束
    for(auto& it : thread_map_){
        if(it.second.joinable()){
            it.second.join();
            std::cout << "join thread: "  << ++i << std::endl;
        }
    }
    if(manager_thread_.joinable()){
        manager_thread_.join(); // 等待管理者线程结束
        std::cout << "destroy manager thread -------" << std::endl;
    }  
}

int ThreadPool::getTaskNum(){
    std::lock_guard<std::mutex> mtx_guard(task_queue__mtx_);
    int t = task_queue_.size();
    return t;
}

void ThreadPool::worker_func(void* arg){

    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    std::cout << std::this_thread::get_id() << "  create !! " << std::endl;

    while(pool->exit_pool_flag_ == false){// 退出标志为true, 退出线程
        std::function<void()> task = nullptr;
        // 如果任务队列为空，且线程池没有退出标志，则等待
        {
            std::unique_lock<std::mutex> mtx_guard(pool->task_queue__mtx_);
            while(pool->task_queue_.empty() && !pool->exit_pool_flag_){
                std::cout << std::this_thread::get_id() << "  wating !! " << std::endl;
                

                pool->cv_.wait(mtx_guard); // 等待唤醒

                // 即使不满足下面的退出条件也要减1，使exit_thread_num_归0，防止后面使其他线程退出
                if(pool->exit_thread_num_ > 0){
                    pool->exit_thread_num_--; 

                    if(pool->live_thread_num_ > pool->min_thread_num_){
                        pool->live_thread_num_--;
                        // std::cout << "destroy one worker thread -------" << std::endl;
                        std::lock_guard<std::mutex> mtx_guard(pool->exited_threads_mtx_);
                        pool->exited_threads_.emplace_back(std::this_thread::get_id());
                        return; // 退出线程
                    }
                }

            }            
            if(!pool->task_queue_.empty()){
                task = std::move(pool->task_queue_.front());// 获取任务 
                // std::cout << "get task" << std::endl;
                pool->task_queue_.pop();         
            }else{
                std::cout << "task queque is empty, get null task" << std::endl;
            }
        }

        if(task != nullptr){
            pool->busy_thread_num_++; // 忙线程数加1
            task(); // 执行任务
            std::cout << "task is done" << std::endl;
            pool->busy_thread_num_--; // 忙线程数减1      
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

void ThreadPool::addTask(std::function<void(void)> task){
    
    if(exit_pool_flag_)
        return;

    {
        std::lock_guard<std::mutex> mtx_guard(task_queue__mtx_);
        task_queue_.emplace(task);
        // std::cout << "add task" << std::endl;
    }

    cv_.notify_one();  // 添加了任务，唤醒线程使其执行任务

}


