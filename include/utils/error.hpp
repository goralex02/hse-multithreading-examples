#pragma once

#include <cerrno>
#include <cstring>
#include <iostream>
#include <source_location>

inline void LogSystemError(std::source_location loc = std::source_location::current()) {
  std::cerr << "Error in " << loc.function_name() << "\n"
            << "  File: " << loc.file_name() << ":" << loc.line() << "\n"
            << "  Reason: " << std::strerror(errno) << " (errno: " << errno
            << ")\n";
}

#define CHECK_ERROR(res)                                                       \
  do {                                                                         \
    if ((res) == -1) {                                                         \
      LogSystemError();                                                        \
      std::exit(EXIT_FAILURE);                                                 \
    }                                                                          \
  } while (0)
