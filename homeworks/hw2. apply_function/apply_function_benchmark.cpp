#include "apply_function.h"

#include <benchmark/benchmark.h>

#include <cmath>
#include <cstdint>
#include <vector>

namespace {

// Легкая функция: почти нет работы на элемент.
void LightTransform(int& x) {
    x += 1;
    benchmark::DoNotOptimize(x);
}

// Тяжелая CPU-bound функция.
void HeavyTransform(double& x) {
    double value = x;
    for (int i = 0; i < 1000; ++i) {
        value = std::sin(value) + std::cos(value) + std::sqrt(value + 10.0);
    }
    x = value;
    benchmark::DoNotOptimize(x);
}

}  // namespace

// ------------------------------------------------------------
// Сценарий 1: однопоточная версия должна быть быстрее
// Маленький вектор + очень легкая transform
// ------------------------------------------------------------
static void BM_ApplyFunction_SingleThread_Faster(benchmark::State& state) {
    const int threadCount = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<int> data(1000, 1);

        ApplyFunction<int>(
            data,
            [](int& x) { LightTransform(x); },
            threadCount
        );

        benchmark::ClobberMemory();
    }

    state.SetLabel("small data + light transform");
}

BENCHMARK(BM_ApplyFunction_SingleThread_Faster)->Arg(1)->Arg(2)->Arg(4)->Arg(8);

// ------------------------------------------------------------
// Сценарий 2: многопоточная версия должна быть быстрее
// Большой вектор + тяжелая transform
// ------------------------------------------------------------
static void BM_ApplyFunction_MultiThread_Faster(benchmark::State& state) {
    const int threadCount = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<double> data(200000, 1.2345);

        ApplyFunction<double>(
            data,
            [](double& x) { HeavyTransform(x); },
            threadCount
        );

        benchmark::ClobberMemory();
    }

    state.SetLabel("large data + heavy transform");
}

BENCHMARK(BM_ApplyFunction_MultiThread_Faster)->Arg(1)->Arg(2)->Arg(4)->Arg(8);