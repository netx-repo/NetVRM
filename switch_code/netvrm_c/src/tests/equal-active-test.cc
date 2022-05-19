#include "equal-active-test.hpp"

#include <algorithm>
#include <cmath>

namespace netvrm {
const void EqualActiveTest::reallocate_memory(
    Application::ApplicationVec &inactive_apps,
    Application::ApplicationVec &new_active_apps,
    Application::ApplicationVec &old_active_apps,
    std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_pkts,
    int curr_sec
) noexcept {
    const int num_active_apps = new_active_apps.size() + old_active_apps.size();
    const int app_num_bits = static_cast<int>(ceil(log2(num_active_apps)));
    const int mem = 1 << (options.total_hw - app_num_bits);
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