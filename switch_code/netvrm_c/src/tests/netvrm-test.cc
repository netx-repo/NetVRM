#include "netvrm-test.hpp"
#include "../util.hpp"

#include <algorithm>
#include <cmath>
#include <tuple>
#include <utility>
#include <vector>
#include <array>
#include <memory>


extern "C" {
    #include "tofinopd/netvrm_wan/pd/pd.h"
    #include "bf_switchd/bf_switchd.h"
    #include "tofino/pdfixed/pd_conn_mgr.h"
    #include "tofino/pdfixed/pd_common.h"
    #include "pkt_mgr/pkt_mgr_intf.h"
}

namespace netvrm {
const int NetVRMTest::calc_norm_mem(Application &app, const std::array<int, SWITCH_NUM> &app_pkts) const noexcept {
    float norm_mem = 0;
    const int pkts_count_on_all_switch = std::accumulate(app_pkts.begin(), app_pkts.end(), 0);
    for (const auto& s : switches) {
        const int curr_mem_on_switch = app.switch_map_mem[s.id];
        // this app's packets count on this switch
        const int pkts_count_on_switch = app_pkts[s.id];
        norm_mem += curr_mem_on_switch * static_cast<float>(pkts_count_on_switch) / pkts_count_on_all_switch;
    }

    return std::max(1, static_cast<int>(floor(norm_mem)));
}

const auto NetVRMTest::calc_under_mem_dist(Application &app, const std::array<int, SWITCH_NUM> &app_pkts, int under_mem_factor) const noexcept {
    const int total_pkts_count = std::accumulate(app_pkts.begin(), app_pkts.end(), 0);
    std::array<std::pair<int, int>, SWITCH_NUM> updates;
    for (const auto& s: switches) {
        const int pkts_cnt = app_pkts[s.id];
        // This might not be a difference of powers of 2, so we use this to estimate.
        const int est_under_mem = static_cast<int>(static_cast<float>(under_mem_factor) * pkts_cnt / total_pkts_count);
        const int old_mem = app.switch_map_mem[s.id];
        int mem = old_mem;

        while (mem - old_mem <= est_under_mem) {
            mem *= 2;
        }

        const int under_mem = mem - old_mem;
        updates[s.id] = std::make_pair(mem, under_mem);
    }

    return updates;
}

const auto NetVRMTest::calc_over_mem_dist(Application &app, const std::array<int, SWITCH_NUM> &app_pkts, int over_mem_factor) const noexcept {
    std::array<std::pair<int, int>, SWITCH_NUM> updates;
    if (over_mem_factor <= 0) {
        return updates;
    }

    const int total_pkts_count = std::accumulate(app_pkts.begin(), app_pkts.end(), 0);
    std::array<float, SWITCH_NUM> pkts_dist;
    float inverse_sum;
    int curr_over_mem;
    for (const auto& s: switches) {
        const int pkts_cnt = app_pkts[s.id];
        if ((pkts_cnt == 0) && (app.switch_map_mem[s.id] > options.lowest_mem_inactive_app)) {
            int release_mem = app.switch_map_mem[s.id] - options.lowest_mem_inactive_app;
            assert(release_mem>=0);
            curr_over_mem += release_mem;
            updates[s.id] = std::make_pair(options.lowest_mem_inactive_app, release_mem);
        } else {
            pkts_dist[s.id] = static_cast<float>(pkts_cnt) / total_pkts_count;
            inverse_sum += 1 / pkts_dist[s.id];
        }
    }

    over_mem_factor = over_mem_factor - curr_over_mem;

    for (const auto& s: switches) {
        if (app_pkts[s.id] <= 0) {
            continue;
        }

        const auto est_over_mem_dist = over_mem_factor / (inverse_sum * pkts_dist[s.id]);
        const int old_mem = app.switch_map_mem[s.id];
        int new_mem = old_mem;
        while (old_mem - new_mem <= est_over_mem_dist && new_mem / 2 > options.lowest_mem_inactive_app) {
            new_mem /= 2;
        }

        updates[s.id] = std::make_pair(new_mem, old_mem - new_mem);
    }

    return updates;
}

const void NetVRMTest::reallocate_memory(
    Application::ApplicationVec &inactive_apps,
    Application::ApplicationVec &new_active_apps,
    Application::ApplicationVec &old_active_apps,
    std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_pkts,
    std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_miss,
    int curr_sec
) noexcept {
    // Map of switch_id to a 3-tuple (app_id, target_mem, under_mem)
    ProvisionList under_prov_list;
    // Map of switch_id to a 3-tuple (app_id, target_mem, over_mem)
    ProvisionList over_prov_list;

    for (auto app: old_active_apps) {
        const auto curr_util = app->utility_list.back();
        if (curr_util > app->slo) {
            // HINT: with a longer alloc_epoch, do not need to calculate underprovioned app
            if (curr_sec%options.alloc_epoch == 0) {
                // handles under-provisioned apps
                app->under_provision_count++;
                if (app->under_provision_count >= options.max_under_prov_cycle) {
                    app->dropped = true;
                    std::cout << str_drop_app;
                    app->print_info();
                    std::fill(app->switch_map_mem.begin(), app->switch_map_mem.end(), 0);
                    continue;
                } else {
                    const auto switch_map_pkt_cnt = app_pkts.at(app->id);
                    const int norm_mem = calc_norm_mem(*app, switch_map_pkt_cnt);
                    const int under_mem_total = calc_under_mem_total(*app, norm_mem);
                    const auto mem_update = calc_under_mem_dist(*app, switch_map_pkt_cnt, under_mem_total);
                    for (const auto& s: switches) {
                        const auto tp = mem_update[s.id];
                        under_prov_list[s.id].push_back(std::make_tuple(app->uid, tp.first, tp.second));
                    }
                }
            }
        } else {
            if (curr_sec%options.alloc_epoch == 0) {
                app->under_provision_count = 0;
            }
            if (curr_util == -1) {
                // no traffic for the current application, use the minimum memory
                for (const auto& s: switches) {
                    const int over_mem = app->switch_map_mem[s.id] - options.lowest_mem_inactive_app;
                    over_prov_list[s.id].push_back(std::make_tuple(app->uid, options.lowest_mem_inactive_app, over_mem));
                }
            } else {
                const auto switch_map_pkt_cnt = app_pkts.at(app->id);
                const int norm_mem = calc_norm_mem(*app, switch_map_pkt_cnt);
                const int over_mem_total = calc_over_mem_total(*app, norm_mem);
                const auto mem_update = calc_over_mem_dist(*app, switch_map_pkt_cnt, over_mem_total);

                for (const auto& s: switches) {
                    const auto tp = mem_update[s.id];
                    over_prov_list[s.id].push_back(std::make_tuple(app->uid, tp.first, tp.second));
                }
            }
        }
    }
    std::cout << "calc under_mem_list, over_mem_list finish..." << std::endl;
    for (auto &s: switches) {
        s.ava_mem = total_mem();
        s.app_uid_map_mem.clear();
        for (auto app:old_active_apps) {
            // exclude dropped app
            if (!app->dropped) {
                s.app_uid_map_mem[app->uid] = app->switch_map_mem[s.id];
                s.ava_mem -= app->switch_map_mem[s.id];
            }
        }
    };
    coordinate_realloc(over_prov_list, under_prov_list, inactive_apps, new_active_apps, old_active_apps, curr_sec);
}

auto NetVRMTest::get_app_by_uid(int uid) noexcept {
    return apps_ptr_list[uid];
}

void NetVRMTest::coordinate_realloc(ProvisionList &over_prov_list,
    ProvisionList &under_prov_list,
    Application::ApplicationVec &inactive_apps,
    Application::ApplicationVec &new_active_apps,
    Application::ApplicationVec &old_active_apps,
    int curr_sec) noexcept {

    // remove all the inactive apps
    for (auto app: inactive_apps) {
        std::fill(app->switch_map_mem.begin(), app->switch_map_mem.end(), 0);
        std::cout << str_end_app << "release memory" << app->uid << " " << app->id << std::endl;
    }

    // dyna_realloc between overprovisioned app and underprovisioned app

    for (auto& s: switches) {

        auto &over_prov_list_switch = over_prov_list[s.id];
        auto &under_prov_list_switch = under_prov_list[s.id];
        if (curr_sec%options.alloc_epoch == 0) {
            s.ava_over_mem = std::accumulate(over_prov_list_switch.begin(), over_prov_list_switch.end(), 0, [](const int& a, auto& b)
                {return a +std::get<2>(b);});
        }
        else {
            s.ava_over_mem = 0;
        }
        int ava_mem_total = s.ava_mem + s.ava_over_mem;
        int need_mem = 0;

        std::sort(over_prov_list_switch.begin(), over_prov_list_switch.end(), [](auto& tp1, auto& tp2) { return std::get<2>(tp1) > std::get<2>(tp2); });
        if (curr_sec%options.alloc_epoch == 0) {
            std::sort(under_prov_list_switch.begin(), under_prov_list_switch.end(), [](auto& tp1, auto& tp2) { return std::get<2>(tp1) < std::get<2>(tp2); });
            // check which applications can get under_mem
            for (auto &under_prov_tp : under_prov_list_switch) {
                const int app_uid = std::get<0>(under_prov_tp);
                auto app = get_app_by_uid(app_uid);
                const int target_mem = std::get<1>(under_prov_tp);
                const int under_mem = std::get<2>(under_prov_tp);

                assert(app->switch_map_mem[s.id] == s.app_uid_map_mem[app_uid]);
                if(ava_mem_total >= under_mem) {
                    s.app_uid_map_mem[app_uid] = target_mem;
                    app->switch_map_mem[s.id] = target_mem;
                    ava_mem_total -= under_mem;
                    need_mem += under_mem;
                }
                else {
                    break;
                }
            }
            // check which applications should release over_mem
            while(need_mem > 0) {
                if (need_mem <= s.ava_mem) {
                    //ava_mem can satisfy the total requirements
                    s.ava_mem -= need_mem;
                    need_mem = 0;
                }
                else {
                    need_mem -= s.ava_mem;
                    s.ava_mem = 0;
                    // need to grab memory from over_prov
                    const auto over_mem_it = over_prov_list_switch.begin();
                    const int over_mem = std::get<2>(*over_mem_it);
                    const int target_mem = std::get<1>(*over_mem_it);
                    const int app_uid = std::get<0>(*over_mem_it);
                    auto app = get_app_by_uid(app_uid);

                    s.ava_mem = std::max(over_mem-need_mem, 0);
                    s.app_uid_map_mem[app_uid] = target_mem;
                    app->switch_map_mem[s.id] = target_mem;

                    need_mem -= over_mem;
                    need_mem = std::max(need_mem, 0);
                    over_prov_list_switch.erase(over_mem_it);
                    s.ava_over_mem -= over_mem;
                    // assert(static_cast<int>(over_prov_list_switch.size())>=0);
                }
            }
            assert(s.check_validity(total_mem()));
        }
    }


    // handle new arriving applications
    const auto new_app_mem = total_mem() / options.new_app_mem_frac;
    for (auto app: new_active_apps) {
        for (auto& s: switches) {
            if (s.ava_mem + s.ava_over_mem < new_app_mem) {
                app->rejected = true;
                std::cout << str_rej_app;
                app->print_info();
                break;
            }
        }
        // admit this app only if all the switches have available memory
        if (!app->rejected) {
            std::cout << str_new_app << app->uid << " " << new_app_mem << std::endl;
            for (auto& s: switches) {
                app->switch_map_mem[s.id] = new_app_mem;
                s.app_uid_map_mem[app->uid] = new_app_mem;
                // already sorted
                auto &over_prov_list_switch = over_prov_list[s.id];
                auto &under_prov_list_switch = under_prov_list[s.id];
                int need_mem = new_app_mem;
                // check which applications should release over_mem
                while(need_mem > 0) {
                    if (curr_sec%options.alloc_epoch!=0) {
                        assert(need_mem <= s.ava_mem);
                    }
                    if (need_mem <= s.ava_mem) {
                        //ava_mem can satisfy the total requirements
                        s.ava_mem -= need_mem;
                        need_mem = 0;
                    }
                    else {

                        need_mem -= s.ava_mem;
                        s.ava_mem = 0;
                        // need to grab memory from over_prov
                        assert(static_cast<int>(over_prov_list_switch.size())>0);
                        const auto over_mem_it = over_prov_list_switch.begin();
                        const int over_mem = std::get<2>(*over_mem_it);
                        const int target_mem = std::get<1>(*over_mem_it);
                        const int app_uid = std::get<0>(*over_mem_it);
                        auto app = get_app_by_uid(app_uid);

                        s.ava_mem = std::max(over_mem-need_mem, 0);
                        s.app_uid_map_mem[app_uid] = target_mem;
                        app->switch_map_mem[s.id] = target_mem;

                        need_mem -= over_mem;
                        need_mem = std::max(need_mem, 0);
                        over_prov_list_switch.erase(over_mem_it);
                        s.ava_over_mem -= over_mem;

                    }
                }
            }
        }
    }
}

}
