#include "futex_mutex.hpp"

#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

bool CounterTest() {
    static constexpr int ThreadsCount = 4;
    static constexpr int Iterations = 100000;

    FutexMutex mutex;
    int counter = 0;

    {
        std::vector<std::jthread> threads;
        threads.reserve(ThreadsCount);

        for (int t = 0; t < ThreadsCount; ++t) {
            threads.emplace_back([&]() {
                for (int i = 0; i < Iterations; ++i) {
                    LockGuard guard(mutex);
                    ++counter;
                }
            });
        }
    }

    const int expected = ThreadsCount * Iterations;
    std::cout << std::format(
        "[CounterTest] counter = {}, expected = {}\n",
        counter,
        expected
    );

    return counter == expected;
}

bool MutualExclusionTest() {
    static constexpr int ThreadsCount = 8;
    static constexpr int Iterations = 50000;

    FutexMutex mutex;
    std::atomic<int> inside{0};
    std::atomic<bool> failed{false};

    {
        std::vector<std::jthread> threads;
        threads.reserve(ThreadsCount);

        for (int t = 0; t < ThreadsCount; ++t) {
            threads.emplace_back([&]() {
                for (int i = 0; i < Iterations; ++i) {
                    mutex.lock();

                    int now = inside.fetch_add(1, std::memory_order_relaxed) + 1;
                    if (now > 1) {
                        failed.store(true, std::memory_order_relaxed);
                    }

                    std::this_thread::yield();

                    inside.fetch_sub(1, std::memory_order_relaxed);
                    mutex.unlock();
                }
            });
        }
    }

    std::cout << std::format(
        "[MutualExclusionTest] result = {}\n",
        failed.load() ? "FAILED" : "OK"
    );

    return !failed.load();
}

bool TryLockTest() {
    FutexMutex mutex;
    std::atomic<bool> secondLocked{false};

    mutex.lock();

    {
        std::jthread thread([&]() {
            if (mutex.try_lock()) {
                secondLocked.store(true, std::memory_order_relaxed);
                mutex.unlock();
            }
        });
    }

    bool failedFirstPhase = secondLocked.load(std::memory_order_relaxed);

    mutex.unlock();

    bool secondPhase = mutex.try_lock();
    if (secondPhase) {
        mutex.unlock();
    }

    std::cout << std::format(
        "[TryLockTest] locked while busy = {}, locked after unlock = {}\n",
        failedFirstPhase ? "YES" : "NO",
        secondPhase ? "YES" : "NO"
    );

    return !failedFirstPhase && secondPhase;
}

int main() {
    const bool counterOk = CounterTest();
    const bool mutexOk = MutualExclusionTest();
    const bool tryLockOk = TryLockTest();

    const bool allOk = counterOk && mutexOk && tryLockOk;

    std::cout << std::format(
        "\nOverall result: {}\n",
        allOk ? "ALL TESTS PASSED" : "SOME TESTS FAILED"
    );

    return allOk ? 0 : 1;
}