#include "UnqPtr.h"
#include "ShrdPtr.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace {

struct TestFailure : std::runtime_error {
    using std::runtime_error::runtime_error;
};

void Check(bool condition, const char* expression, int line) {
    if (!condition) {
        throw TestFailure(std::string("Line ") + std::to_string(line) +
                          ": " + expression);
    }
}

#define CHECK(expression) Check(static_cast<bool>(expression), #expression, __LINE__)

struct Lifetime {
    inline static int alive = 0;
    inline static int destroyed = 0;

    explicit Lifetime(int n = 0) : value(n) { ++alive; }
    Lifetime(const Lifetime&) = delete;
    Lifetime& operator=(const Lifetime&) = delete;
    ~Lifetime() { --alive; ++destroyed; }

    int value;

    static void Clear() {
        alive = 0;
        destroyed = 0;
    }
};

struct Base {
    inline static int destroyed = 0;

    explicit Base(int n = 0) : value(n) {}
    virtual ~Base() { ++destroyed; }

    int value;
};

struct Derived : Base {
    inline static int destroyed = 0;

    explicit Derived(int n = 0) : Base(n) {}
    ~Derived() override { ++destroyed; }
};

// Compile-time checks: these are expected restrictions of the provided classes.
static_assert(!std::is_copy_constructible_v<UnqPtr<int>>);
static_assert(!std::is_copy_assignable_v<UnqPtr<int>>);
static_assert(std::is_move_constructible_v<UnqPtr<int>>);
static_assert(std::is_move_assignable_v<UnqPtr<int>>);
static_assert(std::is_copy_constructible_v<ShrdPtr<int>>);
static_assert(std::is_copy_assignable_v<ShrdPtr<int>>);
static_assert(!std::is_move_constructible_v<ShrdPtr<int>>);
static_assert(!std::is_move_assignable_v<ShrdPtr<int>>);
static_assert(std::is_constructible_v<UnqPtr<Base>, UnqPtr<Derived>&&>);
static_assert(std::is_assignable_v<UnqPtr<Base>&, UnqPtr<Derived>&&>);
static_assert(!std::is_constructible_v<UnqPtr<Derived>, UnqPtr<Base>&&>);
static_assert(!std::is_assignable_v<UnqPtr<Derived>&, UnqPtr<Base>&&>);
static_assert(std::is_constructible_v<ShrdPtr<Base>, const ShrdPtr<Derived>&>);
static_assert(std::is_assignable_v<ShrdPtr<Base>&, const ShrdPtr<Derived>&>);
static_assert(!std::is_constructible_v<ShrdPtr<Derived>, const ShrdPtr<Base>&>);
static_assert(!std::is_assignable_v<ShrdPtr<Derived>&, const ShrdPtr<Base>&>);

void UnqEmptyAndAccess() {
    UnqPtr<int> empty;
    UnqPtr<int> nullPointer(nullptr);
    CHECK(empty.Get() == nullptr);
    CHECK(!empty);
    CHECK(nullPointer.Get() == nullptr);
    CHECK(!nullPointer);

    UnqPtr<int> fromValue(42);
    CHECK(fromValue);
    CHECK(*fromValue == 42);
    *fromValue = 24;
    CHECK(*fromValue == 24);
    CHECK(fromValue.Get() != nullptr);

    const UnqPtr<int>& readOnly = fromValue;
    CHECK(*readOnly == 24);
    CHECK(readOnly.Get() == fromValue.Get());

    UnqPtr<Base> object(new Base(17));
    CHECK(object->value == 17);
    const UnqPtr<Base>& readOnlyObject = object;
    CHECK(readOnlyObject->value == 17);
}

void UnqMoveAndRelease() {
    Lifetime::Clear();
    {
        UnqPtr<Lifetime> source(new Lifetime(10));
        Lifetime* address = source.Get();
        UnqPtr<Lifetime> target(std::move(source));
        CHECK(!source);
        CHECK(target.Get() == address);
        CHECK(target->value == 10);

        UnqPtr<Lifetime> replacement(new Lifetime(20));
        CHECK(Lifetime::alive == 2);
        replacement = std::move(target);
        CHECK(!target);
        CHECK(replacement.Get() == address);
        CHECK(Lifetime::alive == 1);
        CHECK(Lifetime::destroyed == 1);

        UnqPtr<Lifetime>& sameObject = replacement;
        replacement = std::move(sameObject);
        CHECK(replacement.Get() == address);

        Lifetime* raw = replacement.Release();
        CHECK(!replacement);
        CHECK(raw == address);
        CHECK(Lifetime::alive == 1);
        delete raw;
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 2);
}

void UnqResetSwapAndLifetime() {
    Lifetime::Clear();
    {
        UnqPtr<Lifetime> first(new Lifetime(1));
        UnqPtr<Lifetime> second(new Lifetime(2));
        Lifetime* original = first.Get();

        first.Reset(original);  // Reset with the same pointer is a no-op.
        CHECK(Lifetime::alive == 2);
        first.Swap(second);
        CHECK(first->value == 2);
        CHECK(second->value == 1);
        first.Swap(first);
        CHECK(first->value == 2);

        first.Reset(new Lifetime(3));
        CHECK(first->value == 3);
        CHECK(Lifetime::destroyed == 1);
        first.Reset();
        CHECK(!first);
        CHECK(Lifetime::alive == 1);
        second.Reset(nullptr);
        CHECK(!second);
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 3);
}

void UnqCompatibleTypes() {
    Base::destroyed = 0;
    Derived::destroyed = 0;
    {
        UnqPtr<Derived> child(new Derived(3));
        UnqPtr<Base> parent(std::move(child));
        CHECK(!child);
        CHECK(parent->value == 3);

        UnqPtr<Derived> secondChild(new Derived(5));
        parent = std::move(secondChild);
        CHECK(!secondChild);
        CHECK(parent->value == 5);
        CHECK(Derived::destroyed == 1);
        CHECK(Base::destroyed == 1);
    }
    CHECK(Derived::destroyed == 2);
    CHECK(Base::destroyed == 2);
}

void ShrdEmptyAndTransfer() {
    Lifetime::Clear();
    {
        ShrdPtr<Lifetime> empty;
        ShrdPtr<Lifetime> nullPointer(nullptr);
        CHECK(!empty && !nullPointer);
        CHECK(empty.UseCount() == 0 && nullPointer.UseCount() == 0);
        CHECK(!empty.Unique());
        CHECK(empty.Get() == nullptr);

        UnqPtr<Lifetime> owner(new Lifetime(11));
        Lifetime* address = owner.Get();
        ShrdPtr<Lifetime> shared(std::move(owner));
        CHECK(!owner);
        CHECK(shared.Get() == address);
        CHECK(shared->value == 11);
        CHECK(shared.UseCount() == 1);
        CHECK(shared.Unique());
        const ShrdPtr<Lifetime>& readOnly = shared;
        CHECK(readOnly.Get() == address);
        CHECK(readOnly->value == 11);
        CHECK((*readOnly).value == 11);

        UnqPtr<Lifetime> emptyOwner;
        ShrdPtr<Lifetime> sharedEmpty(std::move(emptyOwner));
        CHECK(!sharedEmpty);
        CHECK(sharedEmpty.UseCount() == 0);
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 1);
}

void ShrdCopyAssignmentAndLifetime() {
    Lifetime::Clear();
    {
        ShrdPtr<Lifetime> first(UnqPtr<Lifetime>(new Lifetime(7)));
        {
            ShrdPtr<Lifetime> second(first);
            CHECK(first.Get() == second.Get());
            CHECK(first.UseCount() == 2);
            CHECK(!first.Unique());

            ShrdPtr<Lifetime> third;
            third = second;
            CHECK(first.UseCount() == 3);
            CHECK(third.UseCount() == 3);
            third = third;
            CHECK(third.UseCount() == 3);

            ShrdPtr<Lifetime> old(UnqPtr<Lifetime>(new Lifetime(8)));
            CHECK(Lifetime::alive == 2);
            old = first;
            CHECK(Lifetime::destroyed == 1);
            CHECK(old.UseCount() == 4);
        }
        CHECK(first.UseCount() == 1);
        CHECK(first.Unique());
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 2);
}

void ShrdResetAndSwap() {
    Lifetime::Clear();
    {
        ShrdPtr<Lifetime> first(UnqPtr<Lifetime>(new Lifetime(1)));
        ShrdPtr<Lifetime> copy(first);
        ShrdPtr<Lifetime> other(UnqPtr<Lifetime>(new Lifetime(2)));
        first.Swap(other);
        CHECK(first->value == 2);
        CHECK(first.UseCount() == 1);
        CHECK(other->value == 1);
        CHECK(other.UseCount() == 2);
        CHECK(copy.UseCount() == 2);
        first.Swap(first);
        CHECK(first->value == 2);

        first.Reset(new Lifetime(3));
        CHECK(first->value == 3);
        CHECK(Lifetime::destroyed == 1);
        Lifetime* same = first.Get();
        first.Reset(same);
        CHECK(first.Get() == same);
        CHECK(first.UseCount() == 1);

        UnqPtr<Lifetime> newOwner(new Lifetime(4));
        first.Reset(std::move(newOwner));
        CHECK(!newOwner);
        CHECK(first->value == 4);
        CHECK(Lifetime::destroyed == 2);

        other.Reset();
        CHECK(!other);
        CHECK(other.UseCount() == 0);
        CHECK(copy.UseCount() == 1);
        copy.Reset();
        CHECK(!copy);
        CHECK(Lifetime::destroyed == 3);

        first.Reset();
        CHECK(!first);
        CHECK(Lifetime::destroyed == 4);
        first.Reset(); // Repeated reset of an empty pointer.
        CHECK(first.UseCount() == 0);
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 4);
}

void ShrdCompatibleTypes() {
    Base::destroyed = 0;
    Derived::destroyed = 0;
    {
        ShrdPtr<Derived> child(UnqPtr<Derived>(new Derived(21)));
        ShrdPtr<Base> parent(child);
        CHECK(child.UseCount() == 2);
        CHECK(parent.UseCount() == 2);
        CHECK(parent->value == 21);

        ShrdPtr<Base> another(UnqPtr<Base>(new Base(99)));
        another = child;
        CHECK(Base::destroyed == 1);
        CHECK(child.UseCount() == 3);
        CHECK(another.UseCount() == 3);
        parent = child; // Assignment when both types already share a control block.
        CHECK(child.UseCount() == 3);
        child.Reset();
        CHECK(parent.UseCount() == 2);
        another.Reset();
        CHECK(parent.UseCount() == 1);
    }
    CHECK(Derived::destroyed == 1);
    CHECK(Base::destroyed == 2);
}

void StressTest() {
    constexpr std::size_t count = 10000;
    Lifetime::Clear();
    {
        ShrdPtr<Lifetime> root(UnqPtr<Lifetime>(new Lifetime(123)));
        // Array assignment avoids relying on a move constructor for ShrdPtr.
        ShrdPtr<Lifetime>* copies = new ShrdPtr<Lifetime>[count];
        for (std::size_t i = 0; i < count; ++i) {
            copies[i] = root;
        }
        CHECK(root.UseCount() == count + 1);
        CHECK(Lifetime::alive == 1);
        for (std::size_t i = 0; i < count; i += 2) {
            copies[i].Reset();
        }
        CHECK(root.UseCount() == count / 2 + 1);
        delete[] copies;
        CHECK(root.UseCount() == 1);
        CHECK(Lifetime::alive == 1);
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 1);

    Lifetime::Clear();
    for (std::size_t i = 0; i < count; ++i) {
        UnqPtr<Lifetime> owner(new Lifetime(static_cast<int>(i)));
        CHECK(owner->value == static_cast<int>(i));
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == static_cast<int>(count));
}

using TestFunction = void (*)();

struct TestCase {
    const char* name;
    TestFunction function;
};

} // namespace

int RunAllTests() {
    const TestCase tests[] = {
        {"UnqPtr: empty and access", UnqEmptyAndAccess},
        {"UnqPtr: move and release", UnqMoveAndRelease},
        {"UnqPtr: reset, swap, lifetime", UnqResetSwapAndLifetime},
        {"UnqPtr: compatible types", UnqCompatibleTypes},
        {"ShrdPtr: empty and ownership transfer", ShrdEmptyAndTransfer},
        {"ShrdPtr: copy, assignment, lifetime", ShrdCopyAssignmentAndLifetime},
        {"ShrdPtr: reset and swap", ShrdResetAndSwap},
        {"ShrdPtr: compatible types", ShrdCompatibleTypes},
        {"Stress test: ownership and destruction", StressTest}
    };

    int passed = 0;
    int failed = 0;
    for (const TestCase& test : tests) {
        try {
            test.function();
            ++passed;
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cout << "[FAIL] " << test.name << ": " << error.what() << '\n';
        } catch (...) {
            ++failed;
            std::cout << "[FAIL] " << test.name << ": unknown exception\n";
        }
    }

    std::cout << "\nResult: " << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}
