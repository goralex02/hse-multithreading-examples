cmake_minimum_required(VERSION 3.16)
project(futex_mutex_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(futex_mutex tests.cpp)
target_link_libraries(futex_mutex PRIVATE Threads::Threads)