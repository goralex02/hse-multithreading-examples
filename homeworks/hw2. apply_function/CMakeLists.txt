cmake_minimum_required(VERSION 3.16)
project(ApplyFunctionHW CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

add_library(apply_function INTERFACE)
target_include_directories(apply_function INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(apply_function_tests apply_function_tests.cpp)
target_link_libraries(apply_function_tests
    PRIVATE
    apply_function
    GTest::gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(apply_function_tests)

add_executable(apply_function_benchmark apply_function_benchmark.cpp)
target_link_libraries(apply_function_benchmark
    PRIVATE
    apply_function
    benchmark::benchmark
    benchmark::benchmark_main
    Threads::Threads
)