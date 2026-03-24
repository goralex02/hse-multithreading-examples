#include "queue.h"

#include <chrono>
#include <iostream>
#include <thread>

int main() {
    try {
        ipc::ConsumerNode consumer("/mpsc_ring_buffer");
        const uint32_t accepted_type = 1;

        std::cout << "Consumer started. Waiting for type " << accepted_type << std::endl;

        while (true) {
            auto msg = consumer.Recv(accepted_type);
            if (msg.has_value()) {
                std::cout << "Received: "
                          << reinterpret_cast<const char*>(msg->payload.data())
                          << std::endl;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Consumer error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}