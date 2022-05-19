#pragma once

#include <unordered_map>
#include <vector>
#include <tuple>
#include <array>
#include <memory>
#include <chrono>
#include <functional>
#include <string>

#include "../options.hpp"
#include "../application.hpp"
#include "../thread_pool.hpp"
#include "../switch.hpp"

extern "C" {
    #include "tofinopd/netvrm_wan/pd/pd.h"
    #include "bf_switchd/bf_switchd.h"
    #include "tofino/pdfixed/pd_conn_mgr.h"
    #include "tofino/pdfixed/pd_common.h"
}

#define SWITCH_NUM 4
#define THRIFT_PORT_NUM 7777



namespace netvrm {
/* An instance of the test program. */
class Test {
public:
    Test(const Options &options) : options{options} {
        p4_pd_client_init(&sess_hdl);
    }

    static std::unique_ptr<Test> create_test(const Options &options) noexcept;

    // Initializes the state of the switch.
    const void initialize() noexcept;
    const void start() noexcept;

    // WAN setup
    std::array<Switch, 4> switches{{
        Switch(0, {calc_dport(11)}, calc_dport(1), 1),
        Switch(1, {calc_dport(17)}, calc_dport(18), 0),
        Switch(2, {calc_dport(33)}, calc_dport(34), 2),
        Switch(3, {calc_dport(52)}, calc_dport(51), 3),
    }};

protected:
    ThreadPool<p4_pd_sess_hdl_t> thread_pool{[]() {
        p4_pd_sess_hdl_t sess_hdl;
        p4_pd_client_init(&sess_hdl);
        return sess_hdl;
    }};

    /************** CONNECTION-RELATED STRUCTS ******************/
    p4_pd_sess_hdl_t sess_hdl;
    const p4_pd_dev_target_t dev_tgt = {0, PD_DEV_PIPE_ALL};

    const Options options;

    // Virtual switches used in this experiment. The index of a switch in this vector is the switch's id
    // in the NetVRM context.
    inline int calc_dport(int switch_port) {
        if (switch_port >= 1 && switch_port <= 16) {
            return 192-4*switch_port;
        }
        else if (switch_port >= 17 && switch_port <= 32) {
            return 4*switch_port - 68;
        }
        else if (switch_port >= 33 && switch_port <= 40) {
            return 284-4*(switch_port-33);
        }
        else if (switch_port >= 41 && switch_port <= 48) {
            return 316-4*(switch_port-41);
        }
        else if (switch_port >= 49 && switch_port <= 56) {
            return 4*switch_port+220;
        }
        else if (switch_port >= 57 && switch_port <= 64) {
            return 4*switch_port+156;
        }
        else {
            std::cout << "Error: Illegal switch_port" << std::endl;
            return -1;
        }
    }



    // print strings
    std::string str_new_app = "New app ";
    std::string str_drop_app = "Drop app ";
    std::string str_rej_app = "Reject app ";
    std::string str_end_app = "End app ";
    std::string str_main_start = "Main loop start";
    std::string str_main_end = "Main loop end";
    std::string str_trace_end = "Trace end";
    std::string str_exit = "Exit program";

    /* Typedefs for a vector of Application references. */
    using ApplicationVec = std::vector<std::shared_ptr<Application>>;

    ApplicationVec apps_ptr_list;


    // used for pausing the timer when replaying the next trace
    int test_pkts_total = 0;
    const void reset_total_counter() const noexcept;
    const void read_total_counter() noexcept;

    /************** INITIALIZATION STAGE HELPER ***************/
    // Initializes P4 client context and connects to the switch.
    const void init_sess_hdl() noexcept;
    const void insert_pipe_route() const noexcept;
    const void init_stg_dup_entry() const noexcept;
    const void init_util_table_entry() const noexcept;


    /************** GENERAL UTILS ***************/
    // Sets the scope of all tables so that modifications made to tables apply to all pipelines.
    const void couple_pipes() const noexcept;
    // Maximum amount of apps that can be concurrent at a given time.
    inline const int concurrent_active_app() const noexcept {
        return 1 << options.subnet_length;
    }
    // Total memory in slots on each switch.
    inline const int total_mem() const noexcept {
        return 1 << options.total_hw;
    }



    /************** EXPERIMENT RUNTIME HELPER ***************/
    // Enables apps with the given ids.
    const void enable_apps(Application::ApplicationVec &apps) const noexcept;
    // Disables apps with the given ids.
    const void disable_apps(Application::ApplicationVec &apps) const noexcept;
    // Disables all apps.
    const void disable_all_apps() const noexcept;
    // Sets the id field of the app.
    const void set_app_id(Application::ApplicationVec &apps) const noexcept;
    // Adds the provided apps to the set_app_id_table.
    const void add_to_set_app_id_table(Application::ApplicationVec &apps) const noexcept;
    // Removes the provided apps to the set_app_id_table.
    const void remove_from_set_app_id_table(Application::ApplicationVec &apps) const noexcept;
    // Adds the provided apps to the get_hw_offset_table. Additionally, set the offset variable in the app.
    const void add_to_get_hw_offset_table(Application::ApplicationVec &apps) noexcept;
    // Removes the provided apps to the get_hw_offset_table.
    const void remove_from_get_hw_offset_table(Application::ApplicationVec &apps) noexcept;

    // Resets the total_count and miss_count registers for all apps.
    const void reset_app_counter() const noexcept;

    // Reset the virtual stateful memory
    const void reset_virtual_array() const noexcept;
    // Reads the total_count and miss_count registers for all apps.
    const auto read_app_counter() const noexcept;
    // Updates the utilities of the given apps. Returns the app_pkts variable.
    const auto update_utility(Application::ApplicationVec &apps) const noexcept;

    const void calc_sat(Application::ApplicationVec &apps) noexcept;
    // Reallocates the memory for the given applications. This function doesn't commit the result to the switch,
    // nor does it calculate the offset. It simply figures out the memory the app takes in the next round, and
    // puts it in the mem variable in the app. Additionally, for all apps that become inactive, it sets the memory field
    // for them to zero.
    virtual const void reallocate_memory(
        Application::ApplicationVec &inactive_apps,
        Application::ApplicationVec &new_active_apps,
        Application::ApplicationVec &old_active_apps,
        std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_pkts,
        std::unordered_map<int, std::array<int, SWITCH_NUM>> &app_miss,
        int curr_sec
    ) noexcept = 0;

    const void terminate() noexcept;
};

}
