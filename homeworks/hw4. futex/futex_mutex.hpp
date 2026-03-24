#pragma once

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>

inline void FutexWait(void* value, int expectedValue) {
    syscall(SYS_futex, value, FUTEX_WAIT_PRIVATE, expectedValue, nullptr, nullptr, 0);
}

inline void FutexWake(void* value, int count) {
    syscall(SYS_futex, value, FUTEX_WAKE_PRIVATE, count, nullptr, nullptr, 0);
}

class FutexMutex {
public:
    FutexMutex() = default;
    FutexMutex(const FutexMutex&) = delete;
    FutexMutex& operator=(const FutexMutex&) = delete;

    void lock() {
        int expected = 0;
        if (m_state.compare_exchange_strong(
                expected,
                1,
                std::memory_order_acquire,
                std::memory_order_relaxed)) {
            return;
        }

        while (true) {
            if (m_state.exchange(2, std::memory_order_acquire) == 0) {
                return;
            }

            FutexWait(&m_state, 2);
        }
    }

    bool try_lock() {
        int expected = 0;
        return m_state.compare_exchange_strong(
            expected,
            1,
            std::memory_order_acquire,
            std::memory_order_relaxed);
    }

    void unlock() {
        if (m_state.fetch_sub(1, std::memory_order_release) != 1) {
            m_state.store(0, std::memory_order_release);
            FutexWake(&m_state, 1);
        }
    }

private:
    std::atomic<int> m_state{0};
};

class LockGuard {
public:
    explicit LockGuard(FutexMutex& mutex) : m_mutex(mutex) {
        m_mutex.lock();
    }

    ~LockGuard() {
        m_mutex.unlock();
    }

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

private:
    FutexMutex& m_mutex;
};