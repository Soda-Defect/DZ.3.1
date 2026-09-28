#ifndef BENCHMARK_OBSERVER_H
#define BENCHMARK_OBSERVER_H

#include "ShrdPtr.h"
#include "UnqPtr.h"
#include <memory>

// Defined in another translation unit so ordinary optimized builds cannot
// remove the copy/destroy operation while benchmarking it (do not use LTO).
void Observe(const ShrdPtr<int>& pointer);
void Observe(const std::shared_ptr<int>& pointer);
void Observe(const UnqPtr<int>& pointer);
void Observe(const std::unique_ptr<int>& pointer);

#endif
