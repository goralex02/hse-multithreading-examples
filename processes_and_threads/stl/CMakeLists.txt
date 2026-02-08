add_executable(stl_intro stl_intro.cpp)
target_link_libraries(stl_intro PRIVATE Threads::Threads)

add_executable(jthread jthread.cpp)
target_link_libraries(jthread PRIVATE Threads::Threads)