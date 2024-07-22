#include "threadPool.h"

void* taskFunc(void* arg){
    int num = *(int*)arg;
    std::cout << std::this_thread::get_id() << " is working, number = " << num << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return nullptr;
}

void* taskFunc2(void* arg){
    int *num = (int*)arg;
    std::cout << std::this_thread::get_id() << " is working, add " << *num << " + " << *(num+1)
    << " = " << *num + *(num+1) << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return nullptr;
}


int main(){
    std::function<void*(void*)>func = taskFunc;
    std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>(5, 12);
    std::cout << "-------" << std::endl;
    for(int i = 0; i < 100; i++){
        int* arg = new int(i + 100);
        Task t(taskFunc, arg);
        pool->addTask(t);
    }

    for(int i = 0; i < 100; i++){
        int* arg = new int[2];
        arg[0] = i + 100;
        arg[1] = i + 200;
        Task t(taskFunc2, arg);
        pool->addTask(t);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}