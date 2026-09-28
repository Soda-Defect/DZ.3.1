#include "Tests/TestSupport.h"

#include <iostream>
#include <string_view>

void RunUnqTests(TestRunner& runner);
void RunShrdTests(TestRunner& runner);
void RunBenchmarks();

int main(int argc, char* argv[]) {
    if (argc > 2 || (argc == 2 && std::string_view(argv[1]) != "--benchmark")) {
        std::cerr << "Usage: " << argv[0] << " [--benchmark]\n";
        return 2;
    }

    TestRunner runner;
    RunUnqTests(runner);
    RunShrdTests(runner);
    const int result = runner.Result();
    if (result == 0 && argc == 2) RunBenchmarks();
    return result;
}

//g++ -std=c++17 -Wall -Wextra -pedantic -O2 main.cpp Tests/UnqPtrTests.cpp Tests/ShrdPtrTests.cpp Benchmark/benchmarks.cpp Benchmark/BenchmarkObserver.cpp -o pointer_tests
//pointer_tests.exe
//pointer_tests.exe --benchmark