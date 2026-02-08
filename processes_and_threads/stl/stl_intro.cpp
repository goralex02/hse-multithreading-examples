#include <format>
#include <iostream>
#include <thread>

void ThreadFunction(int &argument) {
  std::cout << std::format("[CHILD]: argument value: {}", argument)
            << std::endl;
  argument = 322;
}

int main() {
  int value{42};

  std::thread thread1{ThreadFunction, std::ref(value)};
  thread1.join();
  std::cout << std::format("[CHILD]: argument value after thread1: {}", value)
            << std::endl;

  std::thread thread2{[](int &argument) {
                        std::cout << std::format("[CHILD]: argument value: {}",
                                                 argument)
                                  << std::endl;
                        argument = 1337;
                      },
                      std::ref(value)};
  thread2.join();
  std::cout << std::format("[CHILD]: argument value after thread2: {}", value)
            << std::endl;

  return EXIT_SUCCESS;
}