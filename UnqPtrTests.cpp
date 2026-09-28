#include "UnqPtr.h"
#include "TestSupport.h"

#include <cstddef>
#include <type_traits>
#include <utility>

static_assert(!std::is_copy_constructible_v<UnqPtr<int>>);
static_assert(!std::is_copy_assignable_v<UnqPtr<int>>);
static_assert(std::is_move_constructible_v<UnqPtr<int>>);
static_assert(std::is_move_assignable_v<UnqPtr<int>>);
static_assert(std::is_constructible_v<UnqPtr<Base>, UnqPtr<Derived>&&>);
static_assert(std::is_assignable_v<UnqPtr<Base>&, UnqPtr<Derived>&&>);
static_assert(!std::is_constructible_v<UnqPtr<Derived>, UnqPtr<Base>&&>);
static_assert(!std::is_assignable_v<UnqPtr<Derived>&, UnqPtr<Base>&&>);
static_assert(!std::is_copy_constructible_v<UnqPtr<int[]>>);
static_assert(std::is_move_constructible_v<UnqPtr<int[]>>);

namespace {
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

        first.Reset(original);  
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


void UnqArraySupport() {
    Lifetime::Clear();
    {
        UnqPtr<Lifetime[]> array(new Lifetime[5]);
        CHECK(Lifetime::alive == 5);
        array[0].value = 10;
        array[4].value = 50;
        UnqPtr<Lifetime[]> moved(std::move(array));
        CHECK(!array);
        CHECK(moved[0].value == 10);
        CHECK(moved[4].value == 50);
        CHECK(Lifetime::alive == 5);

        UnqPtr<Lifetime[]> replacement(new Lifetime[2]);
        replacement = std::move(moved);
        CHECK(!moved);
        CHECK(replacement[4].value == 50);
        CHECK(Lifetime::alive == 5);
        CHECK(Lifetime::destroyed == 2);
        Lifetime* raw = replacement.Release();
        CHECK(!replacement);
        delete[] raw;
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 7);
}

void UnqStressTest() {
    constexpr int count = 10000;
    Lifetime::Clear();
    for (int i = 0; i < count; ++i) {
        UnqPtr<Lifetime> owner(new Lifetime(i));
        CHECK(owner->value == i);
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == count);
}
} // namespace

void RunUnqTests(TestRunner& runner) {
    runner.Run("UnqPtr: empty and access", UnqEmptyAndAccess);
    runner.Run("UnqPtr: move and release", UnqMoveAndRelease);
    runner.Run("UnqPtr: reset, swap, lifetime", UnqResetSwapAndLifetime);
    runner.Run("UnqPtr: compatible types", UnqCompatibleTypes);
    runner.Run("UnqPtr: array ownership", UnqArraySupport);
    runner.Run("UnqPtr: repeated ownership", UnqStressTest);
}
