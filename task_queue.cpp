#include "task_queue.h"


TaskQ::TaskQ() = default;

TaskQ::~TaskQ() = default;

int TaskQ::getQueueSize(){

    std::lock_guard<std::mutex> mtx_guard(mtx_);
    int t = task_queue_.size();
    return t;
}

bool TaskQ::addTask(Task &t){
    
    std::lock_guard<std::mutex> mtx_guard(mtx_);
    task_queue_.push(t);

    return true;        
}

bool TaskQ::getTask(Task &t){

    std::lock_guard<std::mutex> mtx_guard(mtx_);
    if(task_queue_.empty()){ // 判断一下是否为空
        return false;
    }        

    t = task_queue_.front();
    task_queue_.pop();

    return true;
}

