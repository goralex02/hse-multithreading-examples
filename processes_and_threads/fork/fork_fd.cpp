#include <utils/error.hpp>

#include <fcntl.h>
#include <unistd.h>

int main() {
  const int fd = open("example.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
  CHECK_ERROR(fd);

  const pid_t childPid = fork();
  CHECK_ERROR(childPid);

  const std::string_view messageToFile =
      childPid != 0 ? "Hello from parent\n" : "Hello from child\n";

  CHECK_ERROR(write(fd, messageToFile.data(), messageToFile.size()));
  CHECK_ERROR(close(fd));

  return EXIT_SUCCESS;
}
