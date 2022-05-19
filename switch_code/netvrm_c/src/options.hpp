#pragma once

#include <cstdlib>
#include <memory>
#include <string>

namespace netvrm {
/* Scheme for memory allocation. */
enum class AllocationScheme {
    /* Equally divide available memory among all applications. */
    EQUAL_ALL,
    /* Equally divide available memory among all active applications. */
    EQUAL_ACTIVE,
    /* NetVRM approach as described in the paper. */
    NETVRM,
};

// The types of applications involved in the experiment.
enum class SceneType {
    HEAVY_HITTER,
    TCP_CONNECTION,
    SUPERSPREADER,
    MIXED,
};

/**
 * Describes the command line arguments passed into the program.
 */
class Options {
public:
    Options(
        const AllocationScheme allocation_scheme,
        const int app_num,
        const int duration,
        const int total_hw,
        const int app_len_mean,
        const int app_len_spread,
        const int stg_dup,
        const SceneType scene_type,
        const int new_app_mem_frac,
        const int random_seed,
        const int subnet_length,
        const int max_under_prov_cycle,
        const int lowest_mem_inactive_app,
        const int mem_cf,
        const double slo,
        const int alloc_epoch
    ) : allocation_scheme{allocation_scheme},
        app_num{app_num},
        duration{duration},
        total_hw{total_hw},
        app_len_mean{app_len_mean},
        app_len_spread{app_len_spread},
        stg_dup{stg_dup},
        scene_type{scene_type},
        new_app_mem_frac{new_app_mem_frac},
        random_seed{random_seed},
        subnet_length{subnet_length},
        max_under_prov_cycle{max_under_prov_cycle},
        lowest_mem_inactive_app{lowest_mem_inactive_app},
        mem_cf{mem_cf},
        slo{slo},
        alloc_epoch{alloc_epoch} {}

    /* In what way the memory allocation is performed. */
    const AllocationScheme allocation_scheme;
    /* Total number of applications to generate. */
    const int app_num;
    /* Base 2 logarithm of the total memory slots per switch in the application. */
    const int total_hw;
    /* The duration in seconds when the applications can arrive. The total length of the experiment is thus duration + max(app.len) */
    const int duration;
    /* The mean of duration of applications in seconds. */
    const int app_len_mean;
    /* The spread of duration of applications in seconds. All applications will have duration in [duration_mean - duration_spread, duration_mean + duration_spread]. */
    const int app_len_spread;
    /* Base 2 logarithm of number of virtual stages in the app. */
    const int stg_dup;
    /* The type of applications to be run in the application. */
    const SceneType scene_type;
    /* Under the NetVRM allocation scheme, when a new app comes in, the fraction of total memory to allocate to it. For instance,
     * if this value is 128, then 1/128 of the total memory will be allocated to the new app.
     */
    const int new_app_mem_frac;
    /* The seed used to generate application traces. */
    const int random_seed;
    /* The number of available subnet addresses in bits that can be allocated to applications. */
    const int subnet_length;
    /* The number of memory allocations cycles where an app can be under-provisioned before being dropped. NetVRM only. */
    const int max_under_prov_cycle;
    /* The lowest memory, in slots, to be allocated to an app if it hasn't received packets for a while. NetVRM only. */
    const int lowest_mem_inactive_app;
    /* Compensation factor used in calculations of over-memory and under-memory factors. NetVRM only. */
    const int mem_cf;

    const double slo;
    const int alloc_epoch;

    static std::unique_ptr<Options> from_cmd_args(const int argc, char **argv) noexcept;
};
}
