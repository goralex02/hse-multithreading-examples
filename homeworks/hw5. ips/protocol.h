#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace ipc {

inline constexpr uint32_t kProtocolVersion = 2;
inline constexpr uint32_t kMagic = 0x49504351; // "IPCQ"

struct SharedState {
    uint32_t magic;
    uint32_t version;
    uint32_t capacity;
    uint32_t reserved;
    std::atomic<size_t> head;
    std::atomic<size_t> tail;
    std::atomic<size_t> ready_tail;
};

struct PacketHeader {
    uint32_t type;
    uint32_t size;
};

}  // namespace ipc