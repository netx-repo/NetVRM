#include "tests/test.hpp"
#include "options.hpp"
#include "thread_pool.hpp"

#include <iostream>
#include <vector>
#include <chrono>
using namespace netvrm;

int main(int argc, char **argv) {
    const auto options = Options::from_cmd_args(argc, argv);
    auto test = Test::create_test(*options);

    test->initialize();
    test->start();

    return 0;
}