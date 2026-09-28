#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

struct TestFailure : std::runtime_error {
    using std::runtime_error::runtime_error;
};

inline void Check(bool condition, const char* expression, int line) {
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

    static void Clear() { alive = destroyed = 0; }
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

class TestRunner {
public:
    void Run(const char* name, void (*function)()) {
        try {
            function();
            ++passed_;
            std::cout << "[PASS] " << name << '\n';
        } catch (const std::exception& error) {
            ++failed_;
            std::cout << "[FAIL] " << name << ": " << error.what() << '\n';
        } catch (...) {
            ++failed_;
            std::cout << "[FAIL] " << name << ": unknown exception\n";
        }
    }

    int Result() const {
        std::cout << "\nResult: " << passed_ << " passed, " << failed_ << " failed.\n";
        return failed_ == 0 ? 0 : 1;
    }

private:
    int passed_ = 0;
    int failed_ = 0;
};

#endif
