#include "../Pointers/ShrdPtr.h"
#include "TestSupport.h"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

static_assert(std::is_copy_constructible_v<ShrdPtr<int>>);
static_assert(std::is_copy_assignable_v<ShrdPtr<int>>);
static_assert(!std::is_move_constructible_v<ShrdPtr<int>>);
static_assert(!std::is_move_assignable_v<ShrdPtr<int>>);
static_assert(std::is_constructible_v<ShrdPtr<Base>, const ShrdPtr<Derived>&>);
static_assert(std::is_assignable_v<ShrdPtr<Base>&, const ShrdPtr<Derived>&>);
static_assert(!std::is_constructible_v<ShrdPtr<Derived>, const ShrdPtr<Base>&>);
static_assert(!std::is_assignable_v<ShrdPtr<Derived>&, const ShrdPtr<Base>&>);
static_assert(!std::is_move_constructible_v<ShrdPtr<int[]>>);

namespace {
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
        first.Reset(); 
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
        parent = child; 
        CHECK(child.UseCount() == 3);
        child.Reset();
        CHECK(parent.UseCount() == 2);
        another.Reset();
        CHECK(parent.UseCount() == 1);
    }
    CHECK(Derived::destroyed == 1);
    CHECK(Base::destroyed == 2);
}


void ShrdStressTest() {
    constexpr std::size_t count = 10000;
    Lifetime::Clear();
    {
        ShrdPtr<Lifetime> root(UnqPtr<Lifetime>(new Lifetime(123)));
        std::vector<ShrdPtr<Lifetime>> copies(count);
        for (auto& copy : copies) copy = root;
        CHECK(root.UseCount() == count + 1);
        CHECK(Lifetime::alive == 1);
        for (std::size_t i = 0; i < count; i += 2) copies[i].Reset();
        CHECK(root.UseCount() == count / 2 + 1);
        copies.clear();
        CHECK(root.UseCount() == 1);
        CHECK(Lifetime::alive == 1);
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 1);
}

void ShrdFactory() {
    Lifetime::Clear();
    {
        auto first = makeShrd<Lifetime>(123);
        CHECK(first->value == 123);
        CHECK(first.UseCount() == 1);
        ShrdPtr<Lifetime> second(first);
        CHECK(second.Get() == first.Get());
        CHECK(first.UseCount() == 2);
    }
    CHECK(Lifetime::alive == 0);
    CHECK(Lifetime::destroyed == 1);
}

void ShrdArraySupport() {
    Lifetime::Clear();
    {
        ShrdPtr<Lifetime[]> first(UnqPtr<Lifetime[]>(new Lifetime[3]));
        CHECK(first.UseCount() == 1);
        CHECK(first.Unique());
        CHECK(Lifetime::alive == 3);
        {
            ShrdPtr<Lifetime[]> second(first);
            CHECK(first.UseCount() == 2);
            second[0].value = 100;
            CHECK(first[0].value == 100);
            second.Reset();
            CHECK(first.UseCount() == 1);
            CHECK(Lifetime::alive == 3);
        }
        first.Reset();
        CHECK(first.UseCount() == 0);
        CHECK(Lifetime::alive == 0);
        CHECK(Lifetime::destroyed == 3);
    }
    CHECK(Lifetime::destroyed == 3);
}
} // namespace

void RunShrdTests(TestRunner& runner) {
    runner.Run("ShrdPtr: empty and ownership transfer", ShrdEmptyAndTransfer);
    runner.Run("ShrdPtr: copy, assignment, lifetime", ShrdCopyAssignmentAndLifetime);
    runner.Run("ShrdPtr: reset and swap", ShrdResetAndSwap);
    runner.Run("ShrdPtr: compatible types", ShrdCompatibleTypes);
    runner.Run("ShrdPtr: many owners", ShrdStressTest);
    runner.Run("ShrdPtr: makeShrd", ShrdFactory);
    runner.Run("ShrdPtr: array ownership", ShrdArraySupport);
}
