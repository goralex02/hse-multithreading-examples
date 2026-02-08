#include <utils/error.hpp>

#include <unistd.h>

#include <format>
#include <iostream>
#include <memory>
#include <thread>

using namespace std::chrono_literals;

int main() {
  int stackVar{42};
  auto heapVar = std::make_unique<int>(1337);

  std::cout << std::format(
                   "[PARENT]: stackVar address: {}, heapVar address: {}",
                   static_cast<const void *>(&stackVar),
                   static_cast<const void *>(heapVar.get()))
            << std::endl;

  const pid_t childPid = fork();
  CHECK_ERROR(childPid);

  if (childPid != 0) {
    std::this_thread::sleep_for(2s);

    std::cout << std::format("[PARENT]: stackVar value: {}, heapVar value: {}",
                             stackVar, *heapVar)
              << std::endl;
  } else {
    std::cout << std::format(
                     "[CHILD]: stackVar address: {}, heapVar address: {}",
                     static_cast<const void *>(&stackVar),
                     static_cast<const void *>(heapVar.get()))
              << std::endl;
    std::cout << std::format("[CHILD]: stackVar value: {}, heapVar value: {}",
                             stackVar, *heapVar)
              << std::endl;

    ++stackVar;
    *heapVar = 322;

    std::cout << std::format(
                     "[CHILD]: stackVar address: {}, heapVar address: {}",
                     static_cast<const void *>(&stackVar),
                     static_cast<const void *>(heapVar.get()))
              << std::endl;
    std::cout << std::format("[CHILD]: stackVar value: {}, heapVar value: {}",
                             stackVar, *heapVar)
              << std::endl;
  }

  return EXIT_SUCCESS;
}