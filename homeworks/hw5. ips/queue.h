#pragma once

#include "protocol.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <vector>

namespace ipc {

class SharedQueueBase {
protected:
    int shm_fd_ = -1;
    size_t mapped_size_ = 0;
    void* mapped_region_ = nullptr;
    SharedState* state_ = nullptr;
    uint8_t* data_ = nullptr;

    void OpenRegion(const char* name, size_t capacity, bool for_creator) {
        if (for_creator) {
            OpenOrCreate(name, capacity);
        } else {
            OpenExisting(name);
        }
    }

    void OpenOrCreate(const char* name, size_t capacity) {
        shm_fd_ = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0666);
        bool created_now = (shm_fd_ != -1);

        if (!created_now) {
            shm_fd_ = shm_open(name, O_RDWR, 0666);
            if (shm_fd_ == -1) {
                throw std::runtime_error("shm_open failed");
            }
        }

        if (created_now) {
            mapped_size_ = sizeof(SharedState) + capacity;
            if (ftruncate(shm_fd_, static_cast<off_t>(mapped_size_)) == -1) {
                close(shm_fd_);
                throw std::runtime_error("ftruncate failed");
            }
        } else {
            struct stat st {};
            if (fstat(shm_fd_, &st) == -1) {
                close(shm_fd_);
                throw std::runtime_error("fstat failed");
            }
            mapped_size_ = static_cast<size_t>(st.st_size);
        }

        mapped_region_ = mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (mapped_region_ == MAP_FAILED) {
            close(shm_fd_);
            throw std::runtime_error("mmap failed");
        }

        state_ = static_cast<SharedState*>(mapped_region_);
        data_ = static_cast<uint8_t*>(mapped_region_) + sizeof(SharedState);

        if (created_now) {
            state_->magic = kMagic;
            state_->version = kProtocolVersion;
            state_->capacity = static_cast<uint32_t>(capacity);
            state_->reserved = 0;
            state_->head.store(0, std::memory_order_relaxed);
            state_->tail.store(0, std::memory_order_relaxed);
            state_->ready_tail.store(0, std::memory_order_relaxed);
        } else {
            if (state_->magic != kMagic || state_->version != kProtocolVersion) {
                throw std::runtime_error("shared memory format mismatch");
            }
        }
    }

    void OpenExisting(const char* name) {
        shm_fd_ = shm_open(name, O_RDWR, 0666);
        if (shm_fd_ == -1) {
            throw std::runtime_error("shm_open failed");
        }

        struct stat st {};
        if (fstat(shm_fd_, &st) == -1) {
            close(shm_fd_);
            throw std::runtime_error("fstat failed");
        }

        mapped_size_ = static_cast<size_t>(st.st_size);

        mapped_region_ = mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
        if (mapped_region_ == MAP_FAILED) {
            close(shm_fd_);
            throw std::runtime_error("mmap failed");
        }

        state_ = static_cast<SharedState*>(mapped_region_);
        data_ = static_cast<uint8_t*>(mapped_region_) + sizeof(SharedState);

        if (state_->magic != kMagic || state_->version != kProtocolVersion) {
            throw std::runtime_error("shared memory format mismatch");
        }
    }

    void CloseRegion() {
        if (mapped_region_ && mapped_region_ != MAP_FAILED) {
            munmap(mapped_region_, mapped_size_);
        }
        if (shm_fd_ != -1) {
            close(shm_fd_);
        }
    }

    void WriteBytes(size_t pos, const void* src, size_t count) {
        const size_t cap = state_->capacity;
        const size_t begin = pos % cap;
        const size_t first = std::min(count, cap - begin);

        std::memcpy(data_ + begin, src, first);
        if (count > first) {
            std::memcpy(data_, static_cast<const uint8_t*>(src) + first, count - first);
        }
    }

    void ReadBytes(size_t pos, void* dst, size_t count) const {
        const size_t cap = state_->capacity;
        const size_t begin = pos % cap;
        const size_t first = std::min(count, cap - begin);

        std::memcpy(dst, data_ + begin, first);
        if (count > first) {
            std::memcpy(static_cast<uint8_t*>(dst) + first, data_, count - first);
        }
    }
};

class ProducerNode : public SharedQueueBase {
public:
    ProducerNode(const char* name, size_t capacity) {
        OpenRegion(name, capacity, true);
    }

    ~ProducerNode() {
        CloseRegion();
    }

    bool Send(uint32_t type, const void* payload, uint32_t payload_size) {
        const size_t total = sizeof(PacketHeader) + payload_size;
        const size_t cap = state_->capacity;

        size_t start = 0;
        while (true) {
            start = state_->tail.load(std::memory_order_relaxed);
            const size_t head = state_->head.load(std::memory_order_acquire);

            if (start + total - head > cap) {
                return false;
            }

            if (state_->tail.compare_exchange_weak(
                    start,
                    start + total,
                    std::memory_order_relaxed,
                    std::memory_order_relaxed)) {
                break;
            }
        }

        PacketHeader hdr{type, payload_size};
        WriteBytes(start, &hdr, sizeof(hdr));
        WriteBytes(start + sizeof(hdr), payload, payload_size);

        while (state_->ready_tail.load(std::memory_order_acquire) != start) {
        }

        state_->ready_tail.store(start + total, std::memory_order_release);
        return true;
    }
};

class ConsumerNode : public SharedQueueBase {
public:
    ConsumerNode(const char* name) {
        OpenRegion(name, 0, false);
    }

    ~ConsumerNode() {
        CloseRegion();
    }

    struct Message {
        uint32_t type;
        std::vector<uint8_t> payload;
    };

    std::optional<Message> Recv(uint32_t wanted_type) {
        while (true) {
            const size_t head = state_->head.load(std::memory_order_relaxed);
            const size_t published = state_->ready_tail.load(std::memory_order_acquire);

            if (head >= published) {
                return std::nullopt;
            }

            PacketHeader hdr{};
            ReadBytes(head, &hdr, sizeof(hdr));
            const size_t total = sizeof(hdr) + hdr.size;

            if (hdr.type == wanted_type) {
                Message result;
                result.type = hdr.type;
                result.payload.resize(hdr.size);
                ReadBytes(head + sizeof(hdr), result.payload.data(), hdr.size);
                state_->head.store(head + total, std::memory_order_release);
                return result;
            }

            state_->head.store(head + total, std::memory_order_release);
        }
    }
};

}  // namespace ipc