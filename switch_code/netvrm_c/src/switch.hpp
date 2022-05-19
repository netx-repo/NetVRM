#pragma once

#include <vector>

namespace netvrm {
/* Represents a virtual switch in the NetVRM runtime. */
class Switch {
public:
    Switch(
        const int id,
        const std::vector<int> in_port,
        const int out_port,
        const int pipe_idx
    ) : id{id},
        in_port{in_port},
        out_port{out_port},
        pipe_idx{pipe_idx} {}

    /* Id of the switch within the NetVRM runtime. */
    const int id;
    /* The in_port on the physical switch that this virtual switch corresponds to. */
    const std::vector<int> in_port;
    /* The out_port on the physical switch that this virtual switch corresponds to. */
    const int out_port;
    /* The pipeline index on the physical switch that this virtual switch corresponds to. */
    const int pipe_idx;
    /* available memory that is not used*/
    int ava_mem;
    /* over_mem from otehr application that can be released*/
    int ava_over_mem;
    std::unordered_map<int, int> app_uid_map_mem;

    inline const bool check_validity(int total_mem) const noexcept {
        int mem = 0;
        if (static_cast<int>(app_uid_map_mem.size()) == 0)
            return true;

        for (auto t: app_uid_map_mem) {
            mem += std::get<1>(t);
        }
        return mem <= total_mem;
    }
};
}
