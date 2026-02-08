#include <utils/error.hpp>

#include <sys/wait.h>
#include <unistd.h>

#include <functional>

void RunDummy(const std::function<void()> &spawner) {
  const pid_t childPid = fork();
  CHECK_ERROR(childPid);

  if (childPid == 0) {
    spawner();

    std::cerr << "spawner failed" << std::endl;
    std::exit(EXIT_FAILURE);
  } else {
    wait(nullptr);
    std::cout << "-----------------------" << std::endl;
  }
}

int main() {
  RunDummy([]() {
    std::cout << "execl: " << std::endl;
    CHECK_ERROR(execl("./dummy", "dummy", "hello", "from", "execl", nullptr));
  });

  RunDummy([]() {
    std::cout << "execv: " << std::endl;
    char *args[] = {(char *)"./dummy", (char *)"vector", (char *)"data",
                    nullptr};
    CHECK_ERROR(execv("./dummy", args));
  });

  RunDummy([]() {
    std::cout << "execvp: " << std::endl;
    char *args[] = {(char *)"dummy", (char *)"found", (char *)"in",
                    (char *)"path", nullptr};
    CHECK_ERROR(execvp("dummy", args));
  });

  RunDummy([]() {
    std::cout << "execve without env override: " << std::endl;
    char *args[] = {(char *)"dummy", (char *)"with", (char *)"env", nullptr};
    CHECK_ERROR(execve("./dummy", args, nullptr));
  });

  RunDummy([]() {
    std::cout << "execve with env override: " << std::endl;
    char *args[] = {(char *)"dummy", (char *)"with", (char *)"env", nullptr};
    char *env[] = {(char *)"EXAMPLE_ENV=322", nullptr};
    CHECK_ERROR(execve("./dummy", args, env));
  });

  return EXIT_SUCCESS;
}