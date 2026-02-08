#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main() {
  {
    std::jthread thread{
        []() { std::cout << "Hello from child thread" << std::endl; }};
  }

  std::jthread thread{[](std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
      std::cout << "Stop is not requested" << std::endl;
      std::this_thread::sleep_for(500ms);
    }

    std::cout << "Stop requested" << std::endl;
  }};

  std::this_thread::sleep_for(2s);
  thread.request_stop();

  return EXIT_SUCCESS;
}