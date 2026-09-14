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

    UnqPtr(const UnqPtr<T>& other);
    UnqPtr(UnqPtr<T>&& other);

    UnqPtr<T>& operator=(const UnqPtr<T>& other);
    UnqPtr<T>& operator=(UnqPtr<T>&& other) noexcept;

    ~UnqPtr();

    T* Get();
    const T* Get() const;
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
UnqPtr<T>::UnqPtr(const UnqPtr<T>& other)
{
    if(other.ptr){
        ptr = new T(*other.ptr);
    }
    else{
        ptr = nullptr;
    }
}

template<typename T>
UnqPtr<T>::UnqPtr(UnqPtr<T>&& other) : ptr(other.ptr)
{
    other.ptr = nullptr;
}

template<typename T>
UnqPtr<T> &UnqPtr<T>::operator=(const UnqPtr<T>& other)
{
    if (this == &other){
        return *this;
    }

    T* newPtr = other.ptr ? new T(*other.ptr) : nullptr;

    delete ptr;
    ptr = newPtr;

    return *this;
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

#endif