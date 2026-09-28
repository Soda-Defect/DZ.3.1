#ifndef BENCHMARK_OBSERVER_H
#define BENCHMARK_OBSERVER_H

#include "../Pointers/ShrdPtr.h"
#include "../Pointers/UnqPtr.h"
#include <memory>

void Observe(const ShrdPtr<int>& pointer);
void Observe(const std::shared_ptr<int>& pointer);
void Observe(const UnqPtr<int>& pointer);
void Observe(const std::unique_ptr<int>& pointer);

#endif
