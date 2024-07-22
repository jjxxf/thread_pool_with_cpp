cmake_minimum_required(VERSION 3.10)  # 必须
set(CMAKE_CXX_STANDARD 20)	# C++14

project(thread_pool)  #工程名

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# include_directories("/usr/local/include/") # 头文件目录

# link_directories("/usr/local/lib/") # 链接库目录

add_executable(${PROJECT_NAME}
main.cpp 
task_queue.cpp
threadPool.cpp) # 生成可执行文件，这里程序名即为功能名

target_link_libraries(${PROJECT_NAME} pthread) # 链接库

