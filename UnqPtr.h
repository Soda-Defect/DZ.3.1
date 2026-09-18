#ifndef UNQPTR_H
#define UNQPTR_H

#include <cstddef>

template<typename T>
class UnqPtr{
private:
    T* ptr;

public:
    UnqPtr();
    UnqPtr(std::nullptr_t);

    explicit UnqPtr(T* ptr_);
    explicit UnqPtr(const T& value);

    UnqPtr(const UnqPtr<T>& other) = delete;
    UnqPtr(UnqPtr<T>&& other) noexcept;

    UnqPtr<T>& operator=(const UnqPtr<T>& other) = delete;
    UnqPtr<T>& operator=(UnqPtr<T>&& other) noexcept;

    ~UnqPtr();

    T* Get();
    const T* Get() const;

    T& operator*();
    const T& operator*() const;

    T* operator->();
    const T* operator->() const;

    operator bool() const;

    void Reset(T* newPtr = nullptr);
    T* Release();

    void Swap(UnqPtr<T>& other);
};

template<typename T>
UnqPtr<T>::UnqPtr() : ptr(nullptr) {}

template<typename T>
UnqPtr<T>::UnqPtr(std::nullptr_t) : ptr(nullptr) {}

template<typename T>
UnqPtr<T>::UnqPtr(T* ptr_) : ptr(ptr_) {}

template<typename T>
UnqPtr<T>::UnqPtr(const T& value) : ptr(new T(value)) {}

template<typename T>
UnqPtr<T>::UnqPtr(UnqPtr<T>&& other) noexcept : ptr(other.ptr)
{
    other.ptr = nullptr;
}

template<typename T>
UnqPtr<T> &UnqPtr<T>::operator=(UnqPtr<T>&& other) noexcept
{
    if (this == &other){
        return *this;
    }

    delete ptr;

    ptr = other.ptr;
    other.ptr = nullptr;

    return *this;
}

template <typename T>
UnqPtr<T>::~UnqPtr()
{
    delete ptr;
}

template <typename T>
T* UnqPtr<T>::Get()
{
    return ptr;
}

template <typename T>
const T* UnqPtr<T>::Get() const
{
    return ptr;
}

template <typename T>
T& UnqPtr<T>::operator*()
{
    return *ptr;
}

template <typename T>
const T& UnqPtr<T>::operator*() const
{
    return *ptr;
}

template <typename T>
T* UnqPtr<T>::operator->()
{
    return ptr;
}

template <typename T>
const T* UnqPtr<T>::operator->() const
{
    return ptr;
}

template <typename T>
UnqPtr<T>::operator bool() const
{
    return ptr != nullptr;
}

template <typename T>
void UnqPtr<T>::Reset(T* newPtr)
{
    if (ptr != newPtr)
    {
        delete ptr;
        ptr = newPtr;
    }
}

template <typename T>
T* UnqPtr<T>::Release()
{
    T* result = ptr;
    ptr = nullptr;

    return result;
}

template <typename T>
void UnqPtr<T>::Swap(UnqPtr<T>& other)
{
    T* temp = ptr;
    ptr = other.ptr;
    other.ptr = temp;
}

#endif