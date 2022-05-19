#include <cstring>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <tuple>
#include <iterator>


#include "test.hpp"
#include "equal-active-test.hpp"
#include "equal-all-test.hpp"
#include "netvrm-test.hpp"
#include "../application.hpp"
#include "../util.hpp"
#include "../macros.hpp"

extern "C" {
    #include "tofinopd/netvrm_wan/pd/pd.h"
    #include "bf_switchd/bf_switchd.h"
    #include "tofino/pdfixed/pd_conn_mgr.h"
    #include "tofino/pdfixed/pd_common.h"
    #include "pkt_mgr/pkt_mgr_intf.h"
    #include "tofino/bf_pal/bf_pal_port_intf.h"
    #include "pkt_mgr/pkt_mgr_intf.h"
}

void init_ports() {
    bf_port_speed_t speed = BF_SPEED_40G;
    bf_fec_type_t fec_type = BF_FEC_TYP_NONE;
    bf_status_t bf_status, bf_status1;
    // int port_list[] = {188, 184, 180, 164, 148, 0, 4, 8, 284, 280, 276, 416, 420, 424, 428};
    // for(int i = 0; i < sizeof(port_list)/sizeof(port_list[0]); i = i + 1 ) {
    //     std::cout << "add ports: " << port_list[i] << std::endl;
    //     bf_status = bf_pal_port_add(0, port_list[i], speed, fec_type);
    // }
    bf_status = bf_pal_port_add_all(0, speed, fec_type);
    bf_status1 = bf_pal_port_enable_all(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

namespace netvrm {
std::unique_ptr<Test> Test::create_test(const Options &options) noexcept {
    std::unique_ptr<bf_switchd_context_t> switchd_main_ctx = std::make_unique<bf_switchd_context_t>();
    switchd_main_ctx->install_dir = std::getenv("SDE_INSTALL");
    char conf_file[100];
    sprintf(conf_file, "%s/share/p4/targets/tofino/netvrm_wan.conf", switchd_main_ctx->install_dir);

    switchd_main_ctx->conf_file = conf_file;
    switchd_main_ctx->skip_p4 = false;
    switchd_main_ctx->skip_port_add = false;
    switchd_main_ctx->running_in_background = true;
    switchd_main_ctx->dev_sts_port = THRIFT_PORT_NUM;
    switchd_main_ctx->dev_sts_thread = true;

    bf_switchd_lib_init(switchd_main_ctx.get());

    switch (options.allocation_scheme) {
        case AllocationScheme::NETVRM:
            return std::make_unique<NetVRMTest>(options);

        case AllocationScheme::EQUAL_ALL:
            return std::make_unique<EqualAllTest>(options);

        case AllocationScheme::EQUAL_ACTIVE:
            return std::make_unique<EqualActiveTest>(options);
    }
}

const void Test::couple_pipes() const noexcept {
    /*
    BATCHED(
        p4_pd_netvrm_wan_read_ht_stg1_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_evict_ht_stg1_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});

        p4_pd_netvrm_wan_read_ht_stg2_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_evict_ht_stg2_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});

        p4_pd_netvrm_wan_read_ht_stg3_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_evict_ht_stg3_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});

        p4_pd_netvrm_wan_read_ht_stg4_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_incr_ht_stg4_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_reset_ht_stg4_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});

        p4_pd_netvrm_wan_read_ht_stg5_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_evict_ht_stg5_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});

        p4_pd_netvrm_wan_read_ht_stg6_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_evict_ht_stg6_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});

        p4_pd_netvrm_wan_read_ht_stg7_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_evict_ht_stg7_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});

        p4_pd_netvrm_wan_read_ht_stg8_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_incr_ht_stg8_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
        p4_pd_netvrm_wan_reset_ht_stg8_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
    ) */
}

const void Test::insert_pipe_route() const noexcept {
    p4_pd_netvrm_wan_pipe_route_set_property(sess_hdl, dev_tgt.device_id, PD_TABLE_ENTRY_SCOPE, {.scope = PD_ENTRY_SCOPE_ALL_PIPELINES}, {.value = 0});
    p4_pd_entry_hdl_t entry_hdl;

    for (auto& s: switches) {
        BATCHED(
            for (auto in_port: s.in_port) {
                auto match_spec = p4_pd_netvrm_wan_pipe_route_match_spec_t({static_cast<uint16_t>(in_port)});
                auto action_spec = p4_pd_netvrm_wan_act_set_egress_action_spec_t({static_cast<uint16_t>(s.out_port)});

                p4_pd_netvrm_wan_pipe_route_table_add_with_act_set_egress(
                    sess_hdl,
                    dev_tgt,
                    &match_spec,
                    &action_spec,
                    &entry_hdl
                );
            }
        )
    }
}

const void Test::init_stg_dup_entry() const noexcept {
    p4_pd_entry_hdl_t entry_hdl;
    uint8_t stg_dup_int = options.stg_dup;

    p4_pd_netvrm_wan_register_write_stg_dup(sess_hdl, dev_tgt, 0, &stg_dup_int);

    BATCHED(
        auto shift_2_match_spec = p4_pd_netvrm_wan_adjust_stg_idx_match_spec_t({0});
        p4_pd_netvrm_wan_adjust_stg_idx_table_add_with_act_stg_idx_shift_2(
            sess_hdl,
            dev_tgt,
            &shift_2_match_spec,
            &entry_hdl
        );

        auto shift_1_match_spec = p4_pd_netvrm_wan_adjust_stg_idx_match_spec_t({1});
        p4_pd_netvrm_wan_adjust_stg_idx_table_add_with_act_stg_idx_shift_1(
            sess_hdl,
            dev_tgt,
            &shift_1_match_spec,
            &entry_hdl
        );

        auto no_op_match_spec = p4_pd_netvrm_wan_adjust_stg_idx_match_spec_t({2});
        p4_pd_netvrm_wan_adjust_stg_idx_table_add_with__no_op(
            sess_hdl,
            dev_tgt,
            &no_op_match_spec,
            &entry_hdl
        );
    )
}

const void Test::init_util_table_entry() const noexcept {
    p4_pd_entry_hdl_t entry_hdl;

    typedef decltype(p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_12) ActionType;
    constexpr auto actions = std::array<std::pair<int, ActionType *>, 13>{
        std::make_pair(4, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_12),
        std::make_pair(5, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_11),
        std::make_pair(6, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_10),
        std::make_pair(7, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_9),
        std::make_pair(8, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_8),
        std::make_pair(9, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_7),
        std::make_pair(10, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_6),
        std::make_pair(11, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_5),
        std::make_pair(12, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_4),
        std::make_pair(13, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_3),
        std::make_pair(14, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_2),
        std::make_pair(15, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_1),
        std::make_pair(1, &p4_pd_netvrm_wan_hash_shift_table_add_with_act_hash_shift_15),
    };

    BATCHED(
        for (const auto tp: actions) {
            const auto params_md_app_hw = tp.first;
            const auto action = tp.second;

            auto match_spec = p4_pd_netvrm_wan_hash_shift_match_spec_t({static_cast<uint8_t>(params_md_app_hw)});
            action(
                sess_hdl,
                dev_tgt,
                &match_spec,
                &entry_hdl
            );
        }
    )
}

const void Test::initialize() noexcept {
    std::cout << "Initialize start..." << std::endl;
    init_ports();
    std::cout << "init_ports end..." << std::endl;
    couple_pipes();
    insert_pipe_route();
    std::cout << "insert_pipe_route end..." << std::endl;
    init_stg_dup_entry();
    std::cout << "init_stg_dup_entry end..." << std::endl;
    init_util_table_entry();
    std::cout << "Initialize end..." << std::endl;
}

const void Test::enable_apps(Application::ApplicationVec &apps) const noexcept {
    uint8_t value = 1;
    BATCHED(
        for (const auto app: apps) {
            p4_pd_netvrm_wan_register_write_isvalid(sess_hdl, dev_tgt, app->id, &value);
        }
    )
}

const void Test::disable_apps(Application::ApplicationVec &apps) const noexcept {
    uint8_t value = 0;
    BATCHED(
        for (const auto app: apps) {
            p4_pd_netvrm_wan_register_write_isvalid(sess_hdl, dev_tgt, app->id, &value);
        }
    )
}

const void Test::disable_all_apps() const noexcept {
    p4_pd_netvrm_wan_register_reset_all_isvalid(sess_hdl, dev_tgt);
}

const void Test::reset_total_counter() const noexcept {
    // p4_pd_netvrm_wan_register_reset_all_counter_bug(sess_hdl, dev_tgt);
}

const void Test::read_total_counter() noexcept {
    test_pkts_total = 0;
    auto num_actually_read_ptr = std::make_unique<int>();
    auto value_cnt_ptr = std::make_unique<int>();

    auto counter_bug_raw_ptr = std::make_unique<p4_pd_netvrm_wan_counter_bug_value_t[]>(SWITCH_NUM);
    p4_pd_netvrm_wan_register_read_counter_bug(
        sess_hdl,
        dev_tgt,
        0,
        REGISTER_READ_HW_SYNC,
        counter_bug_raw_ptr.get(),
        value_cnt_ptr.get()
    );
    std::cout << "total_cnt:";
    for (int i=0; i < SWITCH_NUM; i++) {
        test_pkts_total += counter_bug_raw_ptr[i].f1;
        std::cout << counter_bug_raw_ptr[i].f1 << " ";
    }
    std::cout << std::endl;

    p4_pd_netvrm_wan_register_reset_all_counter_bug(sess_hdl, dev_tgt);
}

const void Test::reset_app_counter() const noexcept {
    BATCHED(
        p4_pd_netvrm_wan_register_reset_all_app_miss_cnt(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_app_pkts_cnt(sess_hdl, dev_tgt);
    )
}

const void Test::reset_virtual_array() const noexcept {
    BATCHED(
        p4_pd_netvrm_wan_register_reset_all_ht_stg1(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_ht_stg2(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_ht_stg3(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_ht_stg4(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_ht_stg5(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_ht_stg6(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_ht_stg7(sess_hdl, dev_tgt);
        p4_pd_netvrm_wan_register_reset_all_ht_stg8(sess_hdl, dev_tgt);
    )
}
const auto Test::read_app_counter() const noexcept {
    std::unordered_map<int, std::array<int, SWITCH_NUM>> app_pkts;
    std::unordered_map<int, std::array<int, SWITCH_NUM>> app_misses;

    auto num_actually_read_ptr = std::make_unique<int>();
    auto value_cnt_ptr = std::make_unique<int>();

    BATCHED(
        auto pkts_cnt_raw_ptr = std::make_unique<p4_pd_netvrm_wan_app_pkts_cnt_value_t[]>(concurrent_active_app() * SWITCH_NUM);
        p4_pd_netvrm_wan_register_range_read_app_pkts_cnt(
            sess_hdl,
            dev_tgt,
            0,
            concurrent_active_app(),
            REGISTER_READ_HW_SYNC,
            num_actually_read_ptr.get(),
            pkts_cnt_raw_ptr.get(),
            value_cnt_ptr.get()
        );

        auto miss_cnt_raw_ptr = std::make_unique<p4_pd_netvrm_wan_app_miss_cnt_value_t[]>(concurrent_active_app() * SWITCH_NUM);
        p4_pd_netvrm_wan_register_range_read_app_miss_cnt(
            sess_hdl,
            dev_tgt,
            0,
            concurrent_active_app(),
            REGISTER_READ_HW_SYNC,
            num_actually_read_ptr.get(),
            miss_cnt_raw_ptr.get(),
            value_cnt_ptr.get()
        );
    )

    for (int app_id = 0; app_id < concurrent_active_app(); app_id++) {
        for (auto& s : switches) {
            const auto pipe_idx = s.pipe_idx;
            const auto pkts_cnt = pkts_cnt_raw_ptr[SWITCH_NUM * app_id + pipe_idx];
            const auto miss_cnt = miss_cnt_raw_ptr[SWITCH_NUM * app_id + pipe_idx];

            app_pkts[app_id][s.id] = pkts_cnt.f1;
            app_misses[app_id][s.id] = miss_cnt.f1;
        }
    }

    return std::make_pair(app_pkts, app_misses);
}

const void Test::add_to_set_app_id_table(Application::ApplicationVec &apps) const noexcept {
    for (const auto& s: switches) {
        p4_pd_entry_hdl_t entry_hdl;
        for (const auto inport: s.in_port) {
            for (const auto app: apps) {
                auto match_spec = p4_pd_netvrm_wan_set_app_id_match_spec_t({
                    static_cast<uint32_t>((app->subnet << 2) + s.id), static_cast<uint16_t>(inport)
                });
                auto action_spec = p4_pd_netvrm_wan_act_set_app_id_action_spec_t({
                    static_cast<uint16_t>(app->id), static_cast<uint8_t>(app->type), static_cast<uint8_t>(s.id)
                });
                p4_pd_netvrm_wan_set_app_id_table_add_with_act_set_app_id(
                    sess_hdl,
                    dev_tgt,
                    &match_spec,
                    &action_spec,
                    &entry_hdl
                );
            }
        }
    }
}

const void Test::remove_from_set_app_id_table(Application::ApplicationVec &apps) const noexcept {
    BATCHED(
        for (const auto& s: switches) {
            for (const auto inport: s.in_port) {
                for (const auto app: apps) {
                    auto match_spec = p4_pd_netvrm_wan_set_app_id_match_spec_t({
                        static_cast<uint32_t>((app->subnet << 2) + s.id), static_cast<uint16_t>(inport)
                    });
                    p4_pd_netvrm_wan_set_app_id_table_delete_by_match_spec(
                        sess_hdl,
                        dev_tgt,
                        &match_spec
                    );
                }
            }
        }
    )

}

const void Test::set_app_id(Application::ApplicationVec &apps) const noexcept {
    for (int i = 0; i < apps.size(); i++) {
        apps[i]->id = i;
    }
}

const void Test::add_to_get_hw_offset_table(Application::ApplicationVec &apps) noexcept {
    std::vector<std::reference_wrapper<Switch>> switches_ref;
    std::transform(std::begin(switches), std::end(switches), std::back_inserter(switches_ref), [](value_type_t<decltype(switches)>& curr_switch) {
        return std::ref(curr_switch);
    });

    thread_pool.add_by_iteratee_and_block<std::reference_wrapper<Switch>>(std::move(switches_ref), [&](p4_pd_sess_hdl_t sess_hdl, value_type_t<decltype(switches_ref)> curr_switch) {
        p4_pd_entry_hdl_t entry_hdl;
        int offset = 0;
        BATCHED(
            for (auto& app: apps) {
                app->switch_map_offset[curr_switch.get().id] = offset;
                auto action_spec = (p4_pd_netvrm_wan_act_get_offset_hw_action_spec_t{static_cast<uint32_t>(offset), static_cast<uint8_t>(log2(app->switch_map_mem[curr_switch.get().id]))});
                switch (curr_switch.get().id) {
                    case 0: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_table_add_with_act_get_offset_hw(sess_hdl, dev_tgt, &match_spec, &action_spec, &entry_hdl);
                        break;
                    }

                    case 1: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_pipe1_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_pipe1_table_add_with_act_get_offset_hw(sess_hdl, dev_tgt, &match_spec, &action_spec, &entry_hdl);
                        break;
                    }

                    case 2: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_pipe2_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_pipe2_table_add_with_act_get_offset_hw(sess_hdl, dev_tgt, &match_spec, &action_spec, &entry_hdl);
                        break;
                    }

                    case 3: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_pipe3_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_pipe3_table_add_with_act_get_offset_hw(sess_hdl, dev_tgt, &match_spec, &action_spec, &entry_hdl);
                        break;
                    }
                }

                offset += app->switch_map_mem[curr_switch.get().id];
            }
        )
        return nullptr;
    });
}

const void Test::remove_from_get_hw_offset_table(Application::ApplicationVec &apps) noexcept {
    std::vector<std::reference_wrapper<Switch>> switches_ref;
    std::transform(std::begin(switches), std::end(switches), std::back_inserter(switches_ref), [](value_type_t<decltype(switches)>& curr_switch) {
        return std::ref(curr_switch);
    });

    thread_pool.add_by_iteratee_and_block(std::move(switches_ref), [&](p4_pd_sess_hdl_t sess_hdl, value_type_t<decltype(switches_ref)> curr_switch) {
        BATCHED(
            for (auto& app: apps) {
                switch (curr_switch.get().id) {
                    case 0: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_table_delete_by_match_spec(sess_hdl, dev_tgt, &match_spec);
                        break;
                    }

                    case 1: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_pipe1_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_pipe1_table_delete_by_match_spec(sess_hdl, dev_tgt, &match_spec);
                        break;
                    }

                    case 2: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_pipe2_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_pipe2_table_delete_by_match_spec(sess_hdl, dev_tgt, &match_spec);
                        break;
                    }

                    case 3: {
                        auto match_spec = p4_pd_netvrm_wan_get_offset_hw_pipe3_match_spec_t{static_cast<uint16_t>(app->id)};
                        p4_pd_netvrm_wan_get_offset_hw_pipe3_table_delete_by_match_spec(sess_hdl, dev_tgt, &match_spec);
                        break;
                    }
                }
            }
        )

        return nullptr;
    });
}

const auto Test::update_utility(Application::ApplicationVec &apps) const noexcept {
    auto tp = read_app_counter();
    auto app_pkts = tp.first;
    auto app_miss = tp.second;
    std::cout << "(app_uid, app_id, evict_ratio, miss_cnt, pkts_cnt)" <<std::endl;
    for (auto app: apps) {
        const auto miss_cnt_by_switch = app_miss.at(app->id);
        const auto pkts_cnt_by_switch = app_pkts.at(app->id);
        const auto miss_cnt = std::accumulate(miss_cnt_by_switch.begin(), miss_cnt_by_switch.end(), 0);
        const auto pkts_cnt = std::accumulate(pkts_cnt_by_switch.begin(), pkts_cnt_by_switch.end(), 0);
        const auto evict_ratio = pkts_cnt == 0 ? -1.0 : static_cast<float>(miss_cnt) / pkts_cnt;
        app->utility_list.push_back(round(evict_ratio*10000.0)/10000.0);
        std::cout << app->uid << " " << app->id << " " << app->utility_list.back() << " " << miss_cnt << " " << pkts_cnt << std::endl;
        app->print_mem();
            std::cout << app->uid << "app_pkts" << pkts_cnt_by_switch[0] << " " << pkts_cnt_by_switch[1]
                << " " << pkts_cnt_by_switch[2] << " " << pkts_cnt_by_switch[3] << std::endl;
                std::cout << app->uid << "app_misses" << miss_cnt_by_switch[0] << " " << miss_cnt_by_switch[1]
                    << " " << miss_cnt_by_switch[2] << " " << miss_cnt_by_switch[3] << std::endl;
    }
    return std::make_pair(app_pkts, app_miss);
}

const void Test::calc_sat(Application::ApplicationVec &apps) noexcept {
    int drop_cnt = 0;
    int reject_cnt = 0;
    for (auto app: apps) {
        std::cout << app->uid << ";";
        if (app->dropped) {
            drop_cnt++;
            std::cout << str_drop_app << std::endl;
        }
        else if (app->rejected) {
            reject_cnt++;
            std::cout << str_rej_app << std::endl;
        }
        else {
            auto cnt_pair = app->calc_sat_cnt();
            std::cout << "cnt_pair;" << cnt_pair.first << ";" << cnt_pair.second << std::endl;
            app->print_history_utility_list();
        }
        assert(!((app->dropped)&&(app->rejected)));
    }
    std::cout << "(dropped, rejected);" << drop_cnt << ";" << reject_cnt <<std::endl;
}

const void Test::terminate() noexcept {
    p4_pd_client_cleanup(sess_hdl);
}

const void Test::start() noexcept {
    std::cout << "generate_applications start..." << std::endl;
    apps_ptr_list = Application::generate_applications(
        options.app_num,
        options.scene_type,
        options.random_seed,
        options.duration,
        options.app_len_mean,
        options.app_len_spread,
        concurrent_active_app(),
        options.slo
    );
    auto apps = apps_ptr_list;
    std::cout << "generate apps finish..." << std::endl;
    auto test_starts = std::chrono::steady_clock::now();
    int curr_sec = 0;
    bool enter_to_pause = true;
    const int experiment_len = std::accumulate(apps.begin(), apps.end(), 0, [&](int prev, value_type_t<decltype(apps)> app) {
        return std::max(app->duration + app->arrival_ts, prev);
    });
    std::cout << "experiment_len:" << experiment_len << std::endl;
    std::cout << str_main_start << std::endl;
    // used for sync with python script
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    while (curr_sec <= experiment_len) {
        auto prev_sec_apps = Application::get_active_applications(apps, curr_sec - 1);
        auto curr_sec_apps = Application::get_active_applications(apps, curr_sec);

        disable_all_apps();
        std::cout << "=============curr_sec" << curr_sec << std::endl;
        auto app_changes = Application::analyze_application_activity_changes(prev_sec_apps, curr_sec_apps);
        auto inactive_apps = app_changes[0];
        auto new_active_apps = app_changes[1];
        auto old_active_apps = app_changes[2];

        auto app_pkts_miss_pair = update_utility(prev_sec_apps);
        auto app_pkts = app_pkts_miss_pair.first;
        auto app_miss = app_pkts_miss_pair.second;
        while (true) {
            read_total_counter();
            std::cout << "test_pkts_total:" << test_pkts_total << std::endl;
            if (test_pkts_total > 100) {
                enter_to_pause = true;
                break;
            }
            else {
                if (curr_sec > 0 && enter_to_pause) {
                    std::cout << str_trace_end << std::endl;
                    enter_to_pause = false;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        reallocate_memory(inactive_apps, new_active_apps, old_active_apps, app_pkts, app_miss, curr_sec);
        remove_from_set_app_id_table(prev_sec_apps);
        remove_from_get_hw_offset_table(prev_sec_apps);

        // recalculate curr_sec_apps to exclude those that are dropped or rejected
        curr_sec_apps = Application::get_active_applications(apps, curr_sec);
        set_app_id(curr_sec_apps);
        add_to_set_app_id_table(curr_sec_apps);
        add_to_get_hw_offset_table(curr_sec_apps);
        reset_app_counter();
        reset_virtual_array();
        enable_apps(curr_sec_apps);
        curr_sec++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    std::cout << str_main_end << std::endl;
    calc_sat(apps);
    std::cout << str_exit << std::endl;
}
}
