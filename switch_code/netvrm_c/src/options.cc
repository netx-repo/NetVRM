#include "options.hpp"
#include <string>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <memory>
#include <experimental/optional>

namespace netvrm {
std::unique_ptr<Options> Options::from_cmd_args(const int argc, char *argv[]) noexcept {
    std::experimental::optional<AllocationScheme> allocation_scheme;
    std::experimental::optional<int> app_num;
    std::experimental::optional<int> duration;
    std::experimental::optional<int> total_hw;
    std::experimental::optional<int> app_len_mean;
    std::experimental::optional<int> app_len_spread;
    std::experimental::optional<int> stg_dup;
    std::experimental::optional<SceneType> scene_type;
    std::experimental::optional<int> new_app_mem_frac;
    std::experimental::optional<int> random_seed;
    std::experimental::optional<int> subnet_length;
    std::experimental::optional<int> max_under_prov_cycle;
    std::experimental::optional<int> lowest_mem_inactive_app;
    std::experimental::optional<int> mem_cf;
    std::experimental::optional<float> slo;
    std::experimental::optional<int> alloc_epoch;

    for (int i = 1; i + 1 < argc; i += 2) {
        if (strcmp(argv[i], "-scheme") == 0) {
            allocation_scheme = static_cast<AllocationScheme>(atoi(argv[i + 1]));
            std::cout << "input scheme:" << atoi(argv[i + 1]) << std::endl;
        } else if (strcmp(argv[i], "-app_num") == 0) {
            app_num = atoi(argv[i + 1]);
            std::cout << "input app_num:" << atoi(argv[i + 1]) << std::endl;
        } else if (strcmp(argv[i], "-duration") == 0) {
            duration = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-total_hw") == 0) {
            total_hw = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-app_len_mean") == 0) {
            app_len_mean = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-app_len_spread") == 0) {
            app_len_spread = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-stg_dup") == 0) {
            stg_dup = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-scene_type") == 0) {
            scene_type = static_cast<SceneType>(atoi(argv[i + 1]));
        } else if (strcmp(argv[i], "-new_app_mem_frac") == 0) {
            new_app_mem_frac = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-random_seed") == 0) {
            random_seed = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-subnet_length") == 0) {
            subnet_length = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-max_under_prov_cycle") == 0) {
            max_under_prov_cycle = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-lowest_mem_inactive_app") == 0) {
            lowest_mem_inactive_app = atoi(argv[i + 1]);
        } else if (strcmp(argv[i], "-mem_cf") == 0) {
            mem_cf = atoi(argv[i + 1]);
            std::cout << "input mem_cf:" << atoi(argv[i + 1]) << std::endl;
        } else if (strcmp(argv[i], "-slo") == 0) {
            slo = atof(argv[i + 1]);
            std::cout << "input slo:" << atof(argv[i + 1]) << std::endl;
        } else if (strcmp(argv[i], "-alloc_epoch") == 0) {
            alloc_epoch = atoi(argv[i + 1]);
            std::cout << "input alloc_epoch:" << atoi(argv[i + 1]) << std::endl;
        }
    }

    return std::make_unique<Options>(
        allocation_scheme.value_or(AllocationScheme::NETVRM),
        app_num.value_or(256),
        duration.value_or(1200),
        total_hw.value_or(16),
        app_len_mean.value_or(600),
        app_len_spread.value_or(240),
        stg_dup.value_or(2),
        scene_type.value_or(SceneType::MIXED),
        new_app_mem_frac.value_or(128),
        random_seed.value_or(0),
        subnet_length.value_or(8),
        max_under_prov_cycle.value_or(4),
        lowest_mem_inactive_app.value_or(16),
        mem_cf.value_or(2),
        slo.value_or(0.02),
        alloc_epoch.value_or(1)
    );
}
}
