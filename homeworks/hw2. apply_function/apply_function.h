#pragma once

#include <algorithm>
#include <functional>
#include <thread>
#include <vector>

template <typename T>
void ApplyFunction(
    std::vector<T>& data,
    const std::function<void(T&)>& transform,
    const int threadCount = 1
) {
    if (data.empty()) {
        return;
    }

    int actualThreadCount = std::max(1, threadCount);
    actualThreadCount = std::min<int>(actualThreadCount, static_cast<int>(data.size()));

    if (actualThreadCount == 1) {
        for (auto& item : data) {
            transform(item);
        }
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(actualThreadCount);

    const std::size_t baseChunk = data.size() / actualThreadCount;
    const std::size_t remainder = data.size() % actualThreadCount;

    std::size_t begin = 0;

    for (int i = 0; i < actualThreadCount; ++i) {
        const std::size_t chunkSize = baseChunk + (static_cast<std::size_t>(i) < remainder ? 1 : 0);
        const std::size_t end = begin + chunkSize;

        threads.emplace_back([begin, end, &data, &transform]() {
            for (std::size_t j = begin; j < end; ++j) {
                transform(data[j]);
            }
        });

        begin = end;
    }

    for (auto& thread : threads) {
        thread.join();
    }
}