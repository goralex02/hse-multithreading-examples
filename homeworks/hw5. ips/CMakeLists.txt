cmake_minimum_required(VERSION 3.16)
project(mpsc_ipc_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(producer producer.cpp)
add_executable(consumer consumer.cpp)

target_link_libraries(producer PRIVATE pthread rt)
target_link_libraries(consumer PRIVATE pthread rt)