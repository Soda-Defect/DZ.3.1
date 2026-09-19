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

    ShrdPtr(UnqPtr<T>&& owner);

    ShrdPtr(const ShrdPtr<T>& other);
    ShrdPtr(ShrdPtr<T>&& other) = delete;

    ShrdPtr<T>& operator=(const ShrdPtr<T>& other);
    ShrdPtr<T>& operator=(ShrdPtr<T>&& other) = delete;

    ~ShrdPtr();

    size_t UseCount();
    bool Unique();

    T* Get();
    const T* Get() const;

    T& operator*();
    const T& operator*() const;

    T* operator->();
    const T* operator->() const;

    operator bool() const;

    void Reset();
    void Reset(T* newPtr);
    void Reset(UnqPtr<T>&& owner);

    void Swap(ShrdPtr<T>& other);
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
ShrdPtr<T>::ShrdPtr(UnqPtr<T>&& owner): master(nullptr), referenceCount(nullptr)
{
    if (owner.Get() == nullptr){
        return;
    }

    size_t* newCount = new size_t(1);
    master = new UnqPtr<T>(std::move(owner));

    referenceCount = newCount;
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
size_t ShrdPtr<T>::UseCount()
{
    if (referenceCount == nullptr){
        return 0;
    }

    return *referenceCount;
}

template <typename T>
bool ShrdPtr<T>::Unique()
{
    return UseCount() == 1;
}

template <typename T>
T* ShrdPtr<T>::Get()
{
    if (master == nullptr){
        return nullptr;
    }

    return master->Get();
}

template <typename T>
const T* ShrdPtr<T>::Get() const
{
    if (master == nullptr){
        return nullptr;
    }

    return master->Get();
}

template <typename T>
T& ShrdPtr<T>::operator*()
{
    return *master->Get();
}

template <typename T>
const T& ShrdPtr<T>::operator*() const
{
    return *master->Get();
}

template <typename T>
T* ShrdPtr<T>::operator->()
{
    return master->Get();
}

template <typename T>
const T* ShrdPtr<T>::operator->() const
{
    return master->Get();
}

template <typename T>
ShrdPtr<T>::operator bool() const
{
    return master != nullptr && master->Get() != nullptr;
}

template <typename T>
void ShrdPtr<T>::Swap(ShrdPtr<T>& other)
{
    UnqPtr<T>* tempMaster = master;
    master = other.master;
    other.master = tempMaster;

    size_t* tempCount = referenceCount;
    referenceCount = other.referenceCount;
    other.referenceCount = tempCount;
}

template <typename T>
void ShrdPtr<T>::Reset()
{
    ReleaseReference();
}

template <typename T>
void ShrdPtr<T>::Reset(T* newPtr)
{
    ShrdPtr<T> temp{UnqPtr<T>(newPtr)};
    Swap(temp);
}

template <typename T>
void ShrdPtr<T>::Reset(UnqPtr<T>&& owner)
{
    ShrdPtr<T> temp(std::move(owner));

    Swap(temp);
}

#endif