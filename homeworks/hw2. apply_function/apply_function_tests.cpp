#include "apply_function.h"

#include <gtest/gtest.h>

#include <atomic>
#include <vector>

TEST(ApplyFunctionTest, EmptyVectorDoesNothing) {
    std::vector<int> data;
    ApplyFunction<int>(data, [](int& x) { x += 1; }, 4);
    EXPECT_TRUE(data.empty());
}

TEST(ApplyFunctionTest, SingleThreadAppliesTransformToAllElements) {
    std::vector<int> data = {1, 2, 3, 4, 5};

    ApplyFunction<int>(data, [](int& x) { x *= 2; }, 1);

    EXPECT_EQ(data, (std::vector<int>{2, 4, 6, 8, 10}));
}

TEST(ApplyFunctionTest, MultipleThreadsApplyTransformToAllElements) {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};

    ApplyFunction<int>(data, [](int& x) { x += 10; }, 3);

    EXPECT_EQ(data, (std::vector<int>{11, 12, 13, 14, 15, 16, 17, 18}));
}

TEST(ApplyFunctionTest, ThreadCountGreaterThanDataSizeIsHandledCorrectly) {
    std::vector<int> data = {10, 20, 30};

    ApplyFunction<int>(data, [](int& x) { --x; }, 10);

    EXPECT_EQ(data, (std::vector<int>{9, 19, 29}));
}

TEST(ApplyFunctionTest, ZeroOrNegativeThreadCountFallsBackToOneThread) {
    std::vector<int> data1 = {1, 2, 3};
    std::vector<int> data2 = {1, 2, 3};

    ApplyFunction<int>(data1, [](int& x) { x += 5; }, 0);
    ApplyFunction<int>(data2, [](int& x) { x += 5; }, -7);

    EXPECT_EQ(data1, (std::vector<int>{6, 7, 8}));
    EXPECT_EQ(data2, (std::vector<int>{6, 7, 8}));
}

TEST(ApplyFunctionTest, WorksWithStrings) {
    std::vector<std::string> data = {"a", "bb", "ccc"};

    ApplyFunction<std::string>(data, [](std::string& s) { s += "!"; }, 2);

    EXPECT_EQ(data, (std::vector<std::string>{"a!", "bb!", "ccc!"}));
}

TEST(ApplyFunctionTest, EveryElementIsProcessedExactlyOnce) {
    constexpr int kSize = 1000;
    std::vector<int> data(kSize, 0);

    ApplyFunction<int>(data, [](int& x) { x += 1; }, 8);

    for (const auto value : data) {
        EXPECT_EQ(value, 1);
    }
}

TEST(ApplyFunctionTest, CanUseAtomicSideEffectForCountingCalls) {
    constexpr int kSize = 500;
    std::vector<int> data(kSize, 1);
    std::atomic<int> calls = 0;

    ApplyFunction<int>(data,
                       [&calls](int& x) {
                           x *= 3;
                           calls.fetch_add(1, std::memory_order_relaxed);
                       },
                       4);

    EXPECT_EQ(calls.load(std::memory_order_relaxed), kSize);

    for (const auto value : data) {
        EXPECT_EQ(value, 3);
    }
}