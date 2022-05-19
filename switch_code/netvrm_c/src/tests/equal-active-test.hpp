#pragma once

#include "test.hpp"

namespace netvrm {
class EqualActiveTest final : public Test {
public:
    EqualActiveTest(const Options &options) : Test(options) {}
protected:
    const void reallocate_memory(
        Application::ApplicationVec &inactive_apps,
        Application::ApplicationVec &new_active_apps,
        Application::ApplicationVec &old_active_apps,
        std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_pkts,
        std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_miss,
        int curr_sec
    ) noexcept final;
};
}
