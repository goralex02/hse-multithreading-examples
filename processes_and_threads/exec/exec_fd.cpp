#include <utils/error.hpp>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

void RunExample(bool closeOnExec = false) {
  int flags = O_WRONLY | O_CREAT | O_APPEND;
  if (closeOnExec) {
    flags |= O_CLOEXEC;
  }

  const int fd = open("example.txt", flags, S_IRWXU);
  CHECK_ERROR(fd);

  const std::string fdAsStr = std::to_string(fd);

  const pid_t childPid = fork();
  CHECK_ERROR(childPid);

  if (childPid == 0) {
    CHECK_ERROR(execl("./writer", "writer", fdAsStr.c_str(), nullptr));
  } else {
    wait(nullptr);
    std::cout << "-----------------------" << std::endl;
  }

  CHECK_ERROR(close(fd));
}

int main() {
  RunExample();
  RunExample(true);
}
