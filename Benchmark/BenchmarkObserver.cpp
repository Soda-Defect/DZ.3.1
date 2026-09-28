#include "BenchmarkObserver.h"

#include <cstdint>

namespace {
volatile std::uintptr_t address = 0;
volatile unsigned int owners = 0;
}

void Observe(const ShrdPtr<int>& pointer) {
    address = reinterpret_cast<std::uintptr_t>(pointer.Get());
    owners = pointer.UseCount();
}

void Observe(const std::shared_ptr<int>& pointer) {
    address = reinterpret_cast<std::uintptr_t>(pointer.get());
    owners = static_cast<unsigned int>(pointer.use_count());
}

void Observe(const UnqPtr<int>& pointer) {
    address = reinterpret_cast<std::uintptr_t>(pointer.Get());
}

void Observe(const std::unique_ptr<int>& pointer) {
    address = reinterpret_cast<std::uintptr_t>(pointer.get());
}
