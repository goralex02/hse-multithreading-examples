#include <utils/error.hpp>

#include <sys/wait.h>
#include <unistd.h>

#include <format>
#include <iostream>

int main() {
  const pid_t childPid = fork();
  CHECK_ERROR(childPid);

  if (childPid != 0) {
    std::cout << std::format("Fork return value is {}", childPid) << std::endl;

    int childStatus{};
    const pid_t exitedProcessPid = wait(&childStatus);
    CHECK_ERROR(exitedProcessPid);

    std::cout << std::format("wait return value: {}, child's exit status: {}",
                             exitedProcessPid, WEXITSTATUS(childStatus))
              << std::endl;
  } else {
    std::cout << std::format("Child pid is {}", getpid()) << std::endl;
    return 123;
  }

  return EXIT_SUCCESS;
}