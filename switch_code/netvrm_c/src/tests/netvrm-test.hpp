#pragma once

#include "test.hpp"

#include <algorithm>

namespace netvrm {
class NetVRMTest final : public Test {
public:
    NetVRMTest(const Options &options) : Test(options) {}
    ~NetVRMTest() {}

    // utility typedefs for clearer function signatures
    using ProvisionList = std::array<std::vector<std::tuple<int, int, int>>, SWITCH_NUM>;
    const void reallocate_memory(
        Application::ApplicationVec &inactive_apps,
        Application::ApplicationVec &new_active_apps,
        Application::ApplicationVec &old_active_apps,
        std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_pkts,
        std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_miss,
        int curr_sec
    ) noexcept final;


private:
    // Calculate the normalized memory of an application using the formula on page 9 in the paper.
    const int calc_norm_mem(Application &app, const std::array<int, SWITCH_NUM> &app_pkts) const noexcept;
    // Calculate the under memory factor of an application using formula 2 in the paper.
    inline const int calc_under_mem_total(Application &app, int norm_mem) const noexcept {
        const auto hit_ratio = std::max(0.01, 1 - app.utility_list.back());
        const auto target_hit_ratio = 1 - app.slo;
        const auto delta_mem = std::pow(target_hit_ratio / hit_ratio, options.mem_cf) * norm_mem - norm_mem;
        const int delta_mem_int = static_cast<int>(delta_mem);
        return delta_mem;
    }
    // Calculate the over memory factor of an application using formula 1 in the paper.
    inline const int calc_over_mem_total(Application &app, int norm_mem) const noexcept {
        const auto hit_ratio = 1 - app.utility_list.back();
        const auto target_hit_ratio = 1 - app.slo;
        auto new_mem = std::pow(target_hit_ratio / hit_ratio, options.mem_cf) * norm_mem;
        const auto delta_mem = norm_mem - new_mem;
        return delta_mem;
    }

    // Calculate how the underprovisioned memory should be allocated to the available switches.
    const auto calc_under_mem_dist(Application &app, const std::array<int, SWITCH_NUM> &app_pkts, int under_mem_factor) const noexcept;
    // Calculate how the overprovisioned memory should be allocated to the available switches.
    const auto calc_over_mem_dist(Application &app, const std::array<int, SWITCH_NUM> &app_pkts, int over_mem_factor) const noexcept;
    // Coordinate the proposed memory distributions from calc_under_mem_dist and calc_over_mem_dist for a final reallocation plan.
    void coordinate_realloc(ProvisionList &over_prov_list,
        ProvisionList &under_prov_list,
        Application::ApplicationVec &inactive_apps,
        Application::ApplicationVec &new_active_apps,
        Application::ApplicationVec &old_active_apps,
        int curr_sec) noexcept;
    // Check if the sum of the given memory allocations on a switch can fit into the physical memory of the switch.
    inline const bool check_mem_dist_validity(std::vector<int> &mems) const noexcept {
        return std::accumulate(mems.begin(), mems.end(), 0) <= total_mem();
    }

    auto get_app_by_uid(int uid) noexcept;
};
}
