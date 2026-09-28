#include "BenchmarkObserver.h"
#include "../Pointers/ShrdPtr.h"
#include "../Pointers/UnqPtr.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

namespace {
constexpr int iterations = 100000;
constexpr int repetitions = 5;

// Storing each address makes the allocations observable to the optimizer.
volatile std::uintptr_t observedAddress = 0;

template <typename Operation>
double Measure(Operation operation) {
    std::vector<double> samples;
    samples.reserve(repetitions);
    operation(); // warm up
    for (int repeat = 0; repeat < repetitions; ++repeat) {
        const auto start = std::chrono::steady_clock::now();
        operation();
        const auto end = std::chrono::steady_clock::now();
        samples.push_back(std::chrono::duration<double, std::nano>(end - start).count()
                          / iterations);
    }
    std::sort(samples.begin(), samples.end());
    return samples[repetitions / 2];
}

void CustomUniqueAllocation() {
    for (int i = 0; i < iterations; ++i) {
        UnqPtr<int> pointer(new int(i));
        observedAddress = reinterpret_cast<std::uintptr_t>(pointer.Get());
    }
}

void StdUniqueAllocation() {
    for (int i = 0; i < iterations; ++i) {
        std::unique_ptr<int> pointer(new int(i));
        observedAddress = reinterpret_cast<std::uintptr_t>(pointer.get());
    }
}

void CustomUniqueMove() {
    UnqPtr<int> source(new int(42));
    for (int i = 0; i < iterations; ++i) {
        UnqPtr<int> target(std::move(source));
        Observe(target);
        source = std::move(target);
    }
}

void StdUniqueMove() {
    std::unique_ptr<int> source(new int(42));
    for (int i = 0; i < iterations; ++i) {
        std::unique_ptr<int> target(std::move(source));
        Observe(target);
        source = std::move(target);
    }
}

void CustomSharedAllocation() {
    for (int i = 0; i < iterations; ++i) {
        ShrdPtr<int> pointer{UnqPtr<int>(new int(i))};
        observedAddress = reinterpret_cast<std::uintptr_t>(pointer.Get());
    }
}

void StdSharedAllocation() {
    for (int i = 0; i < iterations; ++i) {
        std::shared_ptr<int> pointer(new int(i));
        observedAddress = reinterpret_cast<std::uintptr_t>(pointer.get());
    }
}

void CustomSharedCopy() {
    ShrdPtr<int> root{UnqPtr<int>(new int(42))};
    for (int i = 0; i < iterations; ++i) {
        ShrdPtr<int> copy(root);
        Observe(copy);
    }
}

void StdSharedCopy() {
    std::shared_ptr<int> root(new int(42));
    for (int i = 0; i < iterations; ++i) {
        std::shared_ptr<int> copy(root);
        Observe(copy);
    }
}

void PrintComparison(const char* name, void (*custom)(), void (*standard)()) {
    const double customNs = Measure(custom);
    const double standardNs = Measure(standard);
    std::cout << std::left << std::setw(27) << name << std::right
              << std::setw(16) << std::fixed << std::setprecision(1) << customNs
              << std::setw(16) << standardNs
              << std::setw(13) << std::setprecision(2) << customNs / standardNs << '\n';
}
} // namespace

void RunBenchmarks() {
    std::cout << "\nBenchmark: " << iterations << " operations per sample, median of "
              << repetitions << " samples; lower is faster (ns/op).\n"
              << std::left << std::setw(27) << "Operation" << std::right
              << std::setw(16) << "Custom" << std::setw(16) << "std"
              << std::setw(13) << "Ratio" << '\n';
    PrintComparison("Unq: new/delete", CustomUniqueAllocation, StdUniqueAllocation);
    PrintComparison("Unq: move round trip", CustomUniqueMove, StdUniqueMove);
    PrintComparison("Shrd: new/delete", CustomSharedAllocation, StdSharedAllocation);
    PrintComparison("Shrd: copy/destroy", CustomSharedCopy, StdSharedCopy);
}
