#ifndef SHRDPTR_H
#define SHRDPTR_H

#include <cstddef>
#include <utility>
#include "UnqPtr.h"

//предоставляет владение несколькими частями и при этом работать с ресурсами верно

template <typename T>
class ShrdPtr
{
private:
    UnqPtr<T>* master;
    size_t* referenceCount;

    void AddReference();
    void ReleaseReference();

public:
    ShrdPtr();
    ShrdPtr(std::nullptr_t);

    ShrdPtr(const UnqPtr<T>& owner);
    ShrdPtr(UnqPtr<T>&& owner) noexcept;

    ShrdPtr(const ShrdPtr<T>& other);
    ShrdPtr(ShrdPtr<T>&& other) = delete;

    ShrdPtr<T>& operator=(const ShrdPtr<T>& other);
    ShrdPtr<T>& operator=(ShrdPtr<T>&& other) = delete;

    ~ShrdPtr();

    T* Get();
    const T* Get() const;
};

template <typename T>
void ShrdPtr<T>::AddReference()
{
    if (referenceCount != nullptr)
    {
        ++(*referenceCount);
    }
}

template <typename T>
void ShrdPtr<T>::ReleaseReference()
{
    if (referenceCount == nullptr)
        return;

    --(*referenceCount);

    if (*referenceCount == 0)
    {
        delete master;
        delete referenceCount;
    }

    master = nullptr;
    referenceCount = nullptr;
}

template <typename T>
ShrdPtr<T>::ShrdPtr(): master(nullptr), referenceCount(nullptr) {}

template <typename T>
ShrdPtr<T>::ShrdPtr(std::nullptr_t): master(nullptr), referenceCount(nullptr) {}

template <typename T>
ShrdPtr<T>::ShrdPtr(const UnqPtr<T>& owner): master(nullptr), referenceCount(nullptr)
{
    if (owner.Get() != nullptr)
    {
        master = new UnqPtr<T>(owner);
        referenceCount = new size_t(1);
    }
    else
    {
        master = nullptr;
        referenceCount = nullptr;
    }
}

template <typename T>
ShrdPtr<T>::ShrdPtr(UnqPtr<T>&& owner) noexcept : master(nullptr), referenceCount(nullptr)
{
    if (owner.Get() != nullptr)
    {
        master = new UnqPtr<T>(std::move(owner));
        referenceCount = new size_t(1);
    }
    else
    {
        master = nullptr;
        referenceCount = nullptr;
    }
}

template <typename T>
ShrdPtr<T>::ShrdPtr(const ShrdPtr<T>& other) : master(other.master), referenceCount(other.referenceCount)
{
    AddReference();
}

template <typename T>
ShrdPtr<T>& ShrdPtr<T>::operator=(const ShrdPtr<T>& other)
{
    if (this == &other){
        return *this;
    }

    ReleaseReference();

    master = other.master;
    referenceCount = other.referenceCount;

    AddReference();

    return *this;
}

template <typename T>
ShrdPtr<T>::~ShrdPtr()
{
    ReleaseReference();
}

template <typename T>
T* ShrdPtr<T>::Get()
{
    if (master == nullptr)
        return nullptr;

    return master->Get();
}

template <typename T>
const T* ShrdPtr<T>::Get() const
{
    if (master == nullptr)
        return nullptr;

    return master->Get();
}

#endif