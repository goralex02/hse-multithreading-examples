#include <utils/error.hpp>

#include <iostream>
#include <format>

int main(int argc, const char* argv[]) {
    std::cout << "Dummy called with arguments: " << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << std::format("argv[{}]={}", i, argv[i]) << std::endl;
    }

    if (const char* envVar = std::getenv("EXAMPLE_ENV"); envVar) {
        std::cout << std::format("EXAMPLE_ENV={}", envVar) << std::endl;
    } else {
        std::cout <<  "EXAMPLE_ENV is not set" << std::endl;
    }

    return EXIT_SUCCESS;
}