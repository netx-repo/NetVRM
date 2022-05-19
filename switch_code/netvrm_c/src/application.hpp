#pragma once

#include <array>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <iterator>

#include "options.hpp"

extern "C" {
    #include "tofinopd/netvrm_wan/pd/pd.h"
    #include "bf_switchd/bf_switchd.h"
    #include "tofino/pdfixed/pd_conn_mgr.h"
    #include "tofino/pdfixed/pd_common.h"
    #include "pkt_mgr/pkt_mgr_intf.h"
}

#define DEFAULT_APP_SLO 0.02

namespace netvrm {
/* Possible types of applications to be run in the experiment. */
enum class ApplicationType {
    HEAVY_HITTER,
    TCP_CONNECTION,
    SUPERSPREADER,
};

/* An application to be run in the experiment. */
class Application {
public:
    Application(
        const int uid,
        const int id,
        const ApplicationType type,
        const int subnet,
        const int arrival_ts,
        const int duration,
        const double slo
    ) : uid{uid},
        type{type},
        subnet{subnet},
        arrival_ts{arrival_ts},
        duration{duration},
        id{id},
        slo{slo} {}

    /* An id assigned to the application that's unique throughout the entire experiment. */
    int uid;
    /* Type of the application. */
    ApplicationType type;
    /* The index of the app in all switch counters/registers. */
    int id;
    /* The subnet that this application lives in. */
    int subnet;
    /* Utility target of the application. */
    // double slo = DEFAULT_APP_SLO;
    double slo;
    /* Time in second after the experiment starts when the application arrives. */
    int arrival_ts;
    /* Duration in second of how long the application runs. */
    int duration;

    /* The history of utility ratios of the application. This grows by 1 at every memory allocation epoch. */
    std::vector<double> utility_list;

    /* The application's memory, in slots, on each switch. */
    std::array<int, 4> switch_map_mem;
    /* The application's offset on each switch. */
    std::array<int, 4> switch_map_offset;

    /* Whether the application has been dropped, because for four epochs the memory allocation algorithm still can't satisfy the target. */
    bool dropped = false;
    /* Whether the application has been rejected, because the switch is currently handling too much traffic. */
    bool rejected = false;

    /* Returns whether this application is running at the second curr_sec. */
    inline bool is_running(const int curr_sec) const noexcept {
        return arrival_ts <= curr_sec && curr_sec < arrival_ts + duration;
    }
    /* Print basic info */
    inline void print_info() {
        std::cout << "uid:" << uid << " id:" << id << " type:" << static_cast<uint16_t>(type)
            << " subnet:" << subnet << " arrival_ts:" << arrival_ts << " duration:" << duration
            << " dropped:" << dropped << " rejected:" << rejected << std::endl;
    }

    inline void print_mem() {
        std::cout << "mem dist:";
        for (int i =0; i < 4; i++) {
            std::cout << switch_map_mem[i] << " ";
        }
        std::cout << std::endl;
    }

    inline void print_history_utility_list() {
        std::copy(std::begin(utility_list),
              std::end(utility_list),
              std::ostream_iterator<double>(std::cout, " "));
        std::cout << std::endl;
    }

    inline std::pair <int, int> calc_sat_cnt() {
        int sat_cnt = 0;
        int efficient_cnt = 0;
        for (auto& it:utility_list) {
            if (it >= 0) {
                efficient_cnt++;
                if (it <= slo)
                    sat_cnt++;
            }
        }
        return std::make_pair(sat_cnt, efficient_cnt);
    }
    /* Typedefs for a vector of Application references. */
    using ApplicationVec = std::vector<std::shared_ptr<Application>>;
    /* Generates a list of application traces. The list will contain num applications, with types dictated by scene_type. The arrival timestamps and
     * specifically what app has what type is generated with the given seed. Each app is assumed to run for app_duration minutes, and apps arrive throughout
     * arriv_duration minutes.
     *
     * The generated trace will have at most maximum_concurrent_app applications active at a given time.
     */
    static const ApplicationVec generate_applications(const int num, const SceneType scene_type, const int seed, const int arriv_duration,
        const int app_duration_mean, const int app_duration_spread, const int max_concurrent_app, const double input_slo) noexcept;

    /* Returns a vector of applications that are active at a given second and that haven't been rejected.
     *
     * The iterator that is passed in must be sorted by uid.
     */
    static const ApplicationVec get_active_applications(ApplicationVec& apps, const int second_idx) noexcept;

    /* Returns a 3-tuple of vectors of applications representing the apps that 1. are dropped in the new second, 2. become active in the new second, 3. were active in the previous second and are active in the new second.
     *
     * The given vectors must be sorted by uid.
     */
    static const std::array<ApplicationVec, 3> analyze_application_activity_changes(ApplicationVec& prev_sec_apps, ApplicationVec& curr_sec_apps) noexcept;

    /************************* NETVRM ONLY **************************/
    /* For how many memory allocation cycles has this app been under-provisioned. */
    int under_provision_count = 0;

};
}
