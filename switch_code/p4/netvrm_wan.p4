#include <tofino/constants.p4>
#include <tofino/intrinsic_metadata.p4>
#include <tofino/primitives.p4>
#include <tofino/stateful_alu_blackbox.p4>

#include "includes/headers.p4"
#include "includes/parsers.p4"
#include "shared_ht.p4"
#include "shared_ht_ext1.p4"
#include "utils.p4"

action act_notify_ctrl() {
    modify_field(ig_intr_md_for_tm.copy_to_cpu, 1);
    modify_field(ipv4.dstAddr, ht_md.stg2_lo);
}

table notify_ctrl{
    actions {act_notify_ctrl;}
    default_action: act_notify_ctrl();
}

action act_srcaddr_subnet() {
    shift_right(params_md.addr_subnet, ipv4.srcAddr, 22); // match on /10 subnet, used for WAN, each switch sees traffic from /10 subnet
}

table srcaddr_subnet {
    actions {act_srcaddr_subnet;}
    default_action: act_srcaddr_subnet();
}

control hh_ctrl_flow {
    apply(calc_app1_hash_raw);
    apply(set_srcip_key);
    apply(hash_shift);
    apply(calc_ht_idx);

    // go to stg1 and stg2
    apply(evict_ht_stg1);
    if (ht_md.key_hdr==ht_md.stg1_lo) {
        apply(incr_ht_stg2);
    }
    else {
        apply(reset_ht_stg2);
        apply(notify_ctrl);
        apply(incr_app_miss_counter);
        apply(incr_app_miss_tcnt);
    }

    apply(incr_app_pkts_counter);
    apply(incr_app_pkts_tcnt);
    apply(ipv4_route);
}

control opentcp_ctrl_flow {
    if (tcp.ctrl==2){
        shared_flow();
    }
}

control shared_flow {
    apply(calc_app1_hash_raw);
    apply(set_srcip_key);
    apply(hash_shift);
    apply(calc_ht_idx);
    // AVOID duplicate process
    apply(mark_proc);
    // go to stg1 and stg2
    if (params_md.stg_idx == 0) {
        apply(evict_ht_stg1);
        if (ht_md.key_hdr==ht_md.stg1_lo) {
            apply(incr_ht_stg2);
        }
        else if (ht_md.stg1_lo!=0){
            apply(reset_ht_stg2);

            apply(incr_app_miss_counter);
            apply(incr_app_miss_tcnt);
            apply(notify_ctrl);
        }
        apply(incr_app_pkts_counter);
        apply(incr_app_pkts_tcnt);
    }
    // go to stg3 and stg4
    else if (params_md.stg_idx == 1){
        apply(evict_ht_stg3);
        if (ht_md.key_hdr==ht_md.stg3_lo) {
            apply(incr_ht_stg4);
        }
        else if (ht_md.stg3_lo!=0){
            apply(reset_ht_stg4);

            apply(incr_app_miss_counter);
            apply(incr_app_miss_tcnt);
            apply(notify_ctrl);
        }
        apply(incr_app_pkts_counter);
        apply(incr_app_pkts_tcnt);
    }
    // go to stg5 and stg6
    else if (params_md.stg_idx == 2){
        apply(evict_ht_stg5);
        if (ht_md.key_hdr==ht_md.stg5_lo) {
            apply(incr_ht_stg6);
        }
        else if (ht_md.stg5_lo!=0){
            apply(reset_ht_stg6);

            apply(incr_app_miss_counter);
            apply(incr_app_miss_tcnt);
            apply(notify_ctrl);
        }
        apply(incr_app_pkts_counter);
        apply(incr_app_pkts_tcnt);
    }
    else if (params_md.stg_idx == 3){
        apply(evict_ht_stg7);
        if (ht_md.key_hdr==ht_md.stg7_lo) {
            apply(incr_ht_stg8);
        }
        else if (ht_md.stg7_lo!=0){
            apply(reset_ht_stg8);

            apply(incr_app_miss_counter);
            apply(incr_app_miss_tcnt);
            apply(notify_ctrl);
        }
        apply(incr_app_pkts_counter);
        apply(incr_app_pkts_tcnt);
    }
    // apply(ipv4_route);
}

control ip_pair_ctrl_flow {
    apply(calc_ip_hash_raw);
    apply(set_srcip_key_dup);
    apply(hash_shift_dup);
    apply(calc_ht_idx_dup);
    if (params_md.stg_idx == 0) {
        apply(evict_ht_stg1_dup);
        apply(evict_ht_stg2);
        if (ht_md.stg1_lo!=0) {
            if (ipv4.srcAddr!=ht_md.stg1_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
        else if (ht_md.stg2_lo!=0) {
            if (ipv4.dstAddr!=ht_md.stg2_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
    }
    else if (params_md.stg_idx == 1){
        apply(evict_ht_stg3_dup);
        apply(evict_ht_stg4);
        if (ht_md.stg3_lo!=0) {
            if (ipv4.srcAddr!=ht_md.stg3_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
        else if (ht_md.stg4_lo!=0) {
            if (ipv4.dstAddr!=ht_md.stg4_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
    }
    else if (params_md.stg_idx == 2){
        apply(evict_ht_stg5_dup);
        apply(evict_ht_stg6);
        if (ht_md.stg5_lo!=0) {
            if (ipv4.srcAddr!=ht_md.stg5_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
        else if (ht_md.stg6_lo!=0) {
            if (ipv4.dstAddr!=ht_md.stg6_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
    }
    else if (params_md.stg_idx == 3){
        apply(evict_ht_stg7_dup);
        apply(evict_ht_stg8);
        if (ht_md.stg7_lo!=0) {
            if (ipv4.srcAddr!=ht_md.stg7_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
        else if (ht_md.stg8_lo!=0) {
            if (ipv4.dstAddr!=ht_md.stg8_lo) {
                apply(incr_app_miss_counter);
                apply(incr_app_miss_tcnt);
                apply(notify_ctrl);
            }
        }
    }
    apply(incr_app_pkts_counter);
    apply(incr_app_pkts_tcnt);
}

action act_set_egress(egress_spec) {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, egress_spec);
}

action act_set_egress_1() {
    modify_field(ig_intr_md_for_tm.ucast_egress_port, 0);
}

action _drop () {
    drop();
}

table ipv4_route {
    reads {
        ipv4.srcAddr : exact;
        ipv4.dstAddr : exact;
    }
    actions {
        act_set_egress;
        act_set_egress_1;
    }
    default_action : act_set_egress_1();
}

table pipe_route {
    reads {
        ig_intr_md.ingress_port: exact;
    }
    actions {
        act_set_egress;
    }
}

control read_params_ctrl_flow {
    if (ig_intr_md.ingress_port==148) {
        //pipeline0
        apply(get_offset_hw);
    }
    else if (ig_intr_md.ingress_port==0) {
        apply(get_offset_hw_pipe1);
    }
    else if (ig_intr_md.ingress_port==284) {
        apply(get_offset_hw_pipe2);
    }
    else if (ig_intr_md.ingress_port==428) {
        apply(get_offset_hw_pipe3);
    }

    apply(read_valid);
    apply(calc_stg_num);
    apply(read_stg_dup);
    apply(adjust_stg_idx);
}

action act_trans_caida() {
    add_header(ethernet);
    modify_field(ethernet.etherType, ETHERTYPE_VLAN);
    add_header(vlan_tag);
    modify_field(vlan_tag.etherType, ETHERTYPE_IPV4);
}

table trans_caida {
    actions {act_trans_caida;}
    default_action: act_trans_caida();
}

control ingress {
    if (valid(tcp) or valid(udp)) {
        apply(srcaddr_subnet);
        apply(set_app_id);
        read_params_ctrl_flow();
        apply(pipe_route);
    }
}

control egress {
    if ((params_md.isvalid==1) and (params_md.filter_out == 0)) {
        if ((params_md.func_id==0) or (params_md.func_id==1 and valid(tcp))) {
            shared_flow();
        }
        else if (params_md.func_id==2) {
            ip_pair_ctrl_flow();
        }
    }
}
