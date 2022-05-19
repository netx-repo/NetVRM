#include "equal-all-test.hpp"

#include <cmath>

namespace netvrm {
const void EqualAllTest::reallocate_memory(
    Application::ApplicationVec &inactive_apps,
    Application::ApplicationVec &new_active_apps,
    Application::ApplicationVec &old_active_apps,
    std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_pkts,
    std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_miss,
    int curr_sec
) noexcept {
    const int mem = 1 << (options.total_hw - static_cast<int>(log2(concurrent_active_app())));
    if (new_active_apps.size() > 0) {
        for (auto app: new_active_apps) {
            std::cout << str_new_app;
            app->print_info();
            for (const auto& s: switches) {
                app->switch_map_mem[s.id] = mem;
            }
        }
    }

    if (old_active_apps.size() > 0) {
        for (auto app: old_active_apps) {
            for (const auto& s: switches) {
                app->switch_map_mem[s.id] = mem;
            }
        }
    }

    if (inactive_apps.size() > 0) {
        for (auto app: inactive_apps) {
            for (const auto& s: switches) {
                app->switch_map_mem[s.id] = 0;
            }
        }
    }
}

}
