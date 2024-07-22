旧版本，使用queue存线程示例，线程退出后无法销毁线程实例，需在线程池结束时统一销毁

使用c++实现一个线程池，能够自定义限制线程数量，自动根据任务数量调整线程数。

编译与运行方法： 

mkdir build

cd mkdir

cmake ..

make

./thread_pool
