#include <utils/error.hpp>

#include <unistd.h>

#include <iostream>

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    std::cerr << "Invalid input" << std::endl;
    return EXIT_FAILURE;
  }

  constexpr std::string_view Message = "Write from child process\n";

  const int fd = std::stoi(argv[1]);
  CHECK_ERROR(write(fd, Message.data(), Message.size()));
  CHECK_ERROR(close(fd));

  std::cout << "Successfully written to file from child process" << std::endl;

  return EXIT_SUCCESS;
}