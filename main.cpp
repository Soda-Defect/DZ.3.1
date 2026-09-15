#include <iostream>

#include "UnqPtr.h"
#include "ShrdPtr.h"

int main()
{
    UnqPtr<int> u(new int(42));
    ShrdPtr<int> s1(std::move(u));

    ShrdPtr<int> s2 = s1;
    ShrdPtr<int> s3 = s2;

    std::cout << "s1 = " << *s1.Get() << "\n";
    std::cout << "s2 = " << *s2.Get() << "\n";
    std::cout << "s3 = " << *s3.Get() << "\n";

    return 1;
}

//g++ -std=c++11 -Wall -Wextra -o program main.cpp

