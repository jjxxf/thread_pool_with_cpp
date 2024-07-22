#include "threadPool.h"

// 可以根据需求修改任务函数
void taskFunc(int num){
    std::cout << std::this_thread::get_id() << " is working, number = " << num << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void taskFunc2(int num1, int num2){
    std::cout << std::this_thread::get_id() << " is working, add " << num1 << " + " << num2
    << " = " << num1 + num2 << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}


int main(){
    std::shared_ptr<ThreadPool> pool = std::make_shared<ThreadPool>(3, 13); // 创建线程池
    std::cout << "-------" << std::endl;
    for(int i = 0; i < 100; i++){
        auto func = std::bind(taskFunc, i+100); // 绑定任务函数和参数        
        pool->addTask(func); // 添加任务 
    }

    for(int i = 0; i < 100; i++){
        auto func = std::bind(taskFunc2, i+100, i+200);
        pool->addTask(func);
    }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    return 0;
}