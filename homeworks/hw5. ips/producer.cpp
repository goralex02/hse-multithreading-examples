#include "queue.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main() {
    try {
        ipc::ProducerNode producer("/mpsc_ring_buffer", 4096);

        std::string line;
        uint32_t current_type = 1;

        std::cout << "Producer is running. Type messages below:" << std::endl;

        while (std::getline(std::cin, line)) {
            if (producer.Send(current_type, line.c_str(), static_cast<uint32_t>(line.size() + 1))) {
                std::cout << "Sent [" << current_type << "]: " << line << std::endl;
            } else {
                std::cout << "Buffer is full" << std::endl;
            }

            current_type = (current_type == 1 ? 2u : 1u);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Producer error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}