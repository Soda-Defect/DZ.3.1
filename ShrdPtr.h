#ifndef SHRDPTR_H
#define SHRDPTR_H

#include <cstddef>
#include <type_traits>
#include <utility>
#include "UnqPtr.h"

//предоставляет владение несколькими частями и при этом работать с ресурсами верно

template <typename T>
class ShrdPtr
{
private:
    T* ptr;
    size_t* referenceCount;

    void AddReference();
    void ReleaseReference();

    template <typename U>
    friend class ShrdPtr;

public:
    ShrdPtr();
    ShrdPtr(std::nullptr_t);

    ShrdPtr(UnqPtr<T>&& owner);

    ShrdPtr(const ShrdPtr<T>& other);
    ShrdPtr(ShrdPtr<T>&& other) = delete;

    ShrdPtr<T>& operator=(const ShrdPtr<T>& other);
    ShrdPtr<T>& operator=(ShrdPtr<T>&& other) = delete;

    template <typename U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    ShrdPtr(const ShrdPtr<U>& other) noexcept;

    template <typename U, typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
    ShrdPtr<T>& operator=(const ShrdPtr<U>& other) noexcept;

    ~ShrdPtr();

    size_t UseCount() const;
    bool Unique() const;

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
        delete ptr;
        delete referenceCount;
    }

    ptr = nullptr;
    referenceCount = nullptr;
}

template <typename T>
ShrdPtr<T>::ShrdPtr(): ptr(nullptr), referenceCount(nullptr) {}

template <typename T>
ShrdPtr<T>::ShrdPtr(std::nullptr_t): ptr(nullptr), referenceCount(nullptr) {}

template <typename T>
ShrdPtr<T>::ShrdPtr(UnqPtr<T>&& owner): ptr(nullptr), referenceCount(nullptr)
{
    if (owner.Get() == nullptr) {
        return;
    }

    size_t* count = new size_t(1);

    ptr = owner.Release();
    referenceCount = count;
}

template <typename T>
ShrdPtr<T>::ShrdPtr(const ShrdPtr<T>& other) : ptr(other.ptr), referenceCount(other.referenceCount)
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

    ptr = other.ptr;
    referenceCount = other.referenceCount;

    AddReference();

    return *this;
}

template <typename T>
template <typename U, typename>
ShrdPtr<T>::ShrdPtr(const ShrdPtr<U>& other) noexcept : ptr(other.ptr), referenceCount(other.referenceCount)
{
    AddReference();
}

template <typename T>
template <typename U, typename>
ShrdPtr<T>& ShrdPtr<T>::operator=(const ShrdPtr<U>& other) noexcept
{
    ReleaseReference();

    ptr = other.ptr;
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
size_t ShrdPtr<T>::UseCount() const
{
    if (referenceCount == nullptr){
        return 0;
    }

    return *referenceCount;
}

template <typename T>
bool ShrdPtr<T>::Unique() const
{
    return UseCount() == 1;
}

template <typename T>
T* ShrdPtr<T>::Get()
{
    return ptr;
}

template <typename T>
const T* ShrdPtr<T>::Get() const
{
    return ptr;
}

template <typename T>
T& ShrdPtr<T>::operator*()
{
    return *ptr;
}

template <typename T>
const T& ShrdPtr<T>::operator*() const
{
    return *ptr;
}

template <typename T>
T* ShrdPtr<T>::operator->()
{
    return ptr;
}

template <typename T>
const T* ShrdPtr<T>::operator->() const
{
    return ptr;
}

template <typename T>
ShrdPtr<T>::operator bool() const
{
    return ptr != nullptr;
}

template <typename T>
void ShrdPtr<T>::Swap(ShrdPtr<T>& other)
{
    T* tempPtr = ptr;
    ptr = other.ptr;
    other.ptr = tempPtr;

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
    if (ptr == newPtr){
        return;
    }

    UnqPtr<T> owner(newPtr);

    Reset(std::move(owner));
}

template <typename T>
void ShrdPtr<T>::Reset(UnqPtr<T>&& owner)
{
    ShrdPtr<T> temp(std::move(owner));

    Swap(temp);
}

#endif