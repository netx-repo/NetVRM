// RND_MAX = (1<<RND_WIDTH)
#define RND_MAX 4
#define RND_WIDTH 2
#define MAX_CON_TASK 256
#define PARAM_LEN 1024 // PARAM_LEN = MAX_CON_TASK*4

header_type params_md_t {
    fields {
        offset: 32;
        addr_subnet: 32;
        app_hw: 8;
        isvalid: 1;
        filter_out: 1;
        app_id: 16;
        func_id: 8;
        params_offset: 16;
        params_idx: 16;
        switch_id: 4;
        stg_idx: RND_WIDTH;
        stg_dup: 8;
    }
}

metadata params_md_t params_md;

action act_get_offset_hw(offset, hw) {
    modify_field(params_md.offset, offset);
    modify_field(params_md.app_hw, hw);
}

table get_offset_hw {
    reads {
        params_md.app_id: exact;
    }
    actions {
        act_get_offset_hw;
    }
}

table get_offset_hw_pipe1 {
    reads {
        params_md.app_id: exact;
    }
    actions {
        act_get_offset_hw;
    }
}

table get_offset_hw_pipe2 {
    reads {
        params_md.app_id: exact;
    }
    actions {
        act_get_offset_hw;
    }
}

table get_offset_hw_pipe3 {
    reads {
        params_md.app_id: exact;
    }
    actions {
        act_get_offset_hw;
    }
}

// flag to enable or disable network applications
register isvalid {
    width : 1;
    instance_count : MAX_CON_TASK;
}

blackbox stateful_alu salu_read_valid {
    reg : isvalid;
    update_lo_1_value : read_bit;
    output_value : alu_lo;
    output_dst : params_md.isvalid;
}

action act_read_valid() {
    salu_read_valid.execute_stateful_alu(params_md.app_id);
}

table read_valid {
    actions {act_read_valid;}
    default_action: act_read_valid();
}

// stg_dup = 0, 1, 2
register stg_dup {
    width : 8;
    instance_count : 1;
}

blackbox stateful_alu salu_read_stg_dup {
    reg: stg_dup;
    output_value: register_lo;
    output_dst: params_md.stg_dup;
}

action act_read_stg_dup() {
    salu_read_stg_dup.execute_stateful_alu(0);
}

table read_stg_dup {
    actions{act_read_stg_dup;}
    default_action: act_read_stg_dup();
}

// identify app_id based on ip prefix
// func_id = 0 represents HH ,func_id = 1 represents Opentcp
// action act_set_app_id(app_id, func_id, switch_id) {
action act_set_app_id(app_id, func_id, switch_id) {
    modify_field(params_md.app_id, app_id);
    modify_field(params_md.func_id, func_id);
    modify_field(params_md.switch_id, switch_id);
}

action act_set_filter() {
    modify_field(params_md.filter_out, 1);
}

table set_app_id {
    reads{
        params_md.addr_subnet: exact;
        ig_intr_md.ingress_port: exact;
    }
    actions {
        act_set_app_id;
    }
    default_action : act_set_filter();
}

action act_calc_params_idx() {
    add(params_md.params_idx, params_md.params_offset, params_md.app_id);
}

table calc_params_idx {
    actions {act_calc_params_idx;}
    default_action: act_calc_params_idx();
}

field_list stg_idx_hash_field {
    ipv4.srcAddr;
}

field_list_calculation stg_idx_hash {
    input {stg_idx_hash_field;}
    algorithm: crc_8_itu;
    output_width: RND_WIDTH;
}

action act_gen_random() {
    modify_field_with_hash_based_offset(params_md.stg_idx, 0, stg_idx_hash, RND_MAX);
}

table calc_stg_num {
    actions {act_gen_random;}
    default_action: act_gen_random();
}

action _no_op() {
    no_op();
}

table do_no {
    actions {_no_op;}
    default_action: _no_op();
}

action act_stg_idx_shift_1() {
    shift_right(params_md.stg_idx, params_md.stg_idx, 1);
}

action act_stg_idx_shift_2() {
    shift_right(params_md.stg_idx, params_md.stg_idx, 2);
}

table adjust_stg_idx {
    reads {
        params_md.stg_dup: exact;
    }
    actions {
        act_stg_idx_shift_1;
        act_stg_idx_shift_2;
        _no_op;
    }
    default_action: act_stg_idx_shift_2();
}

register app_pkts_cnt {
    width : 64;
    instance_count : MAX_CON_TASK;
}

blackbox stateful_alu salu_incr_app_pkts_counter {
    reg: app_pkts_cnt;

    update_lo_1_value: register_lo+1;
}

action act_incr_app_pkts_counter() {
    salu_incr_app_pkts_counter.execute_stateful_alu(params_md.app_id);
}

table incr_app_pkts_counter {
    actions {act_incr_app_pkts_counter;}
    default_action: act_incr_app_pkts_counter();
}

register app_pkts_total_cnt {
    width : 64;
    instance_count : MAX_CON_TASK;
}

blackbox stateful_alu salu_incr_app_pkts_tcnt {
    reg: app_pkts_total_cnt;

    update_lo_1_value: register_lo+1;
}

action act_incr_app_pkts_tcnt() {
    salu_incr_app_pkts_tcnt.execute_stateful_alu(params_md.app_id);
}

table incr_app_pkts_tcnt {
    actions {act_incr_app_pkts_tcnt;}
    default_action: act_incr_app_pkts_tcnt();
}

register app_miss_cnt {
    width : 64;
    instance_count : MAX_CON_TASK;
}

blackbox stateful_alu salu_incr_app_miss_counter {
    reg: app_miss_cnt;

    update_lo_1_value: register_lo+1;
}

action act_incr_app_miss_counter() {
    salu_incr_app_miss_counter.execute_stateful_alu(params_md.app_id);
}

@pragma stage 11
table incr_app_miss_counter {
    actions {act_incr_app_miss_counter;}
    default_action: act_incr_app_miss_counter();
}

@pragma stage 11
table incr_app_miss_counter_dup1 {
    actions {act_incr_app_miss_counter;}
    default_action: act_incr_app_miss_counter();
}

register app_miss_total_cnt {
    width : 64;
    instance_count : MAX_CON_TASK;
}

blackbox stateful_alu salu_incr_app_miss_tcnt {
    reg: app_miss_total_cnt;

    update_lo_1_value: register_lo+1;
}

action act_incr_app_miss_tcnt() {
    salu_incr_app_miss_tcnt.execute_stateful_alu(params_md.app_id);
}

table incr_app_miss_tcnt {
    actions {act_incr_app_miss_tcnt;}
    default_action: act_incr_app_miss_tcnt();
}

table incr_app_miss_tcnt_dup {
    actions {act_incr_app_miss_tcnt;}
    default_action: act_incr_app_miss_tcnt();
}

// related to hash index assignment

header_type hash_idx_md_t {
    fields {
        app1_hash_raw: REG_LEN_LOG;
        app1_hash2_raw: REG_LEN_LOG;
        app2_hash_raw: REG_LEN_LOG;
        app1_stg_idx: 1;
    }
}

metadata hash_idx_md_t hash_idx_md;

field_list srcip_hash_field {
    ipv4.srcAddr;
}

field_list_calculation srcip_hash {
    input {srcip_hash_field;}
    algorithm: crc32;
    output_width: REG_LEN_LOG;
}

action act_calc_app1_hash_raw() {
    modify_field_with_hash_based_offset(hash_idx_md.app1_hash_raw, 0, srcip_hash, REG_LEN);
}

table calc_app1_hash_raw {
    actions {act_calc_app1_hash_raw;}
    default_action: act_calc_app1_hash_raw();
}

/**
used by count-min sketch
**/
table calc_app1_hash_raw_dup {
    actions {act_calc_app1_hash_raw;}
    default_action: act_calc_app1_hash_raw();
}

field_list_calculation srcip_hash2 {
    input {srcip_hash_field;}
    algorithm: crc32_extend;
    output_width: REG_LEN_LOG;
}

action act_calc_app1_hash2_raw() {
    modify_field_with_hash_based_offset(hash_idx_md.app1_hash2_raw, 0, srcip_hash2, REG_LEN);
}

table calc_app1_hash2_raw {
    actions {act_calc_app1_hash2_raw;}
    default_action: act_calc_app1_hash2_raw();
}

action act_hash2_shift_1() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 1);
}

action act_hash2_shift_2() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 2);
}

action act_hash2_shift_3() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 3);
}

action act_hash2_shift_4() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 4);
}

action act_hash2_shift_5() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 5);
}

action act_hash2_shift_6() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 6);
}

action act_hash2_shift_7() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 7);
}

action act_hash2_shift_8() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 8);
}

action act_hash2_shift_9() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 9);
}

action act_hash2_shift_10() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 10);
}

action act_hash2_shift_11() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 11);
}

action act_hash2_shift_12() {
    shift_right(ht_md.app1_idx_hash2, hash_idx_md.app1_hash2_raw, 12);
}

table hash2_shift {
    reads {
        params_md.app_hw: exact;
    }
    actions {
        act_hash2_shift_1;
        act_hash2_shift_2;
        act_hash2_shift_3;
        act_hash2_shift_4;
        act_hash2_shift_5;
        act_hash2_shift_6;
        act_hash2_shift_7;
        act_hash2_shift_8;
        act_hash2_shift_9;
        act_hash2_shift_10;
        act_hash2_shift_11;
        act_hash2_shift_12;
    }
    default_action: act_hash2_shift_1();
}

/**
END
used by count-min sketch
**/

field_list dstip_hash_field {
    ipv4.dstAddr;
}

field_list_calculation dstip_hash {
    input {dstip_hash_field;}
    algorithm: crc32;
    output_width: REG_LEN_LOG;
}

action act_calc_dstip_hash_raw() {
    modify_field_with_hash_based_offset(hash_idx_md.app1_hash_raw, 0, dstip_hash, REG_LEN);
}

table calc_dstip_hash_raw {
    actions {act_calc_dstip_hash_raw;}
    default_action: act_calc_dstip_hash_raw();
}

field_list ip_hash_field {
    ipv4.srcAddr;
    ipv4.dstAddr;
}

field_list_calculation ip_hash {
    input {ip_hash_field;}
    algorithm: crc32;
    output_width: REG_LEN_LOG;
}

action act_calc_ip_hash_raw() {
    modify_field_with_hash_based_offset(hash_idx_md.app1_hash_raw, 0, ip_hash, REG_LEN);
}

table calc_ip_hash_raw {
    actions {act_calc_ip_hash_raw;}
    default_action: act_calc_ip_hash_raw();
}

field_list_calculation srcip_hash_stg_idx {
    input {srcip_hash_field;}
    algorithm: crc16;
    output_width: 1;
}

action act_calc_app1_stg_idx() {
    modify_field_with_hash_based_offset(hash_idx_md.app1_stg_idx, 0, srcip_hash_stg_idx, 2);
}

table calc_app1_stg_idx{
    actions {act_calc_app1_stg_idx;}
    default_action: act_calc_app1_stg_idx();
}

action act_hash_shift_1() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 1);
}

action act_hash_shift_2() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 2);
}

action act_hash_shift_3() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 3);
}

action act_hash_shift_4() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 4);
}

action act_hash_shift_5() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 5);
}

action act_hash_shift_6() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 6);
}

action act_hash_shift_7() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 7);
}

action act_hash_shift_8() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 8);
}

action act_hash_shift_9() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 9);
}

action act_hash_shift_10() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 10);
}

action act_hash_shift_11() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 11);
}

action act_hash_shift_12() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 12);
}

action act_hash_shift_15() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 15);
}

action act_hash_shift_16() {
    shift_right(ht_md.app1_idx, hash_idx_md.app1_hash_raw, 16);
}

table hash_shift {
    reads {
        params_md.app_hw: exact;
    }
    actions {
        act_hash_shift_1;
        act_hash_shift_2;
        act_hash_shift_3;
        act_hash_shift_4;
        act_hash_shift_5;
        act_hash_shift_6;
        act_hash_shift_7;
        act_hash_shift_8;
        act_hash_shift_9;
        act_hash_shift_10;
        act_hash_shift_11;
        act_hash_shift_12;
        act_hash_shift_15;
        act_hash_shift_16;
    }
    default_action: act_hash_shift_1();
}

table hash_shift_dup {
    reads {
        params_md.app_hw: exact;
    }
    actions {
        act_hash_shift_1;
        act_hash_shift_2;
        act_hash_shift_3;
        act_hash_shift_4;
        act_hash_shift_5;
        act_hash_shift_6;
        act_hash_shift_7;
        act_hash_shift_8;
        act_hash_shift_9;
        act_hash_shift_10;
        act_hash_shift_11;
        act_hash_shift_12;
    }
    default_action: act_hash_shift_1();
}

table hash_shift_dup1 {
    reads {
        params_md.app_hw: exact;
    }
    actions {
        act_hash_shift_1;
        act_hash_shift_2;
        act_hash_shift_3;
        act_hash_shift_4;
        act_hash_shift_5;
        act_hash_shift_6;
        act_hash_shift_7;
        act_hash_shift_8;
        act_hash_shift_9;
        act_hash_shift_10;
        act_hash_shift_11;
        act_hash_shift_12;
    }
    default_action: act_hash_shift_1();
}

action act_calc_ht_idx(){
    add_to_field(ht_md.app1_idx, params_md.offset);
}

table calc_ht_idx {
    actions {act_calc_ht_idx;}
    default_action : act_calc_ht_idx();
}

table calc_ht_idx_dup {
    actions {act_calc_ht_idx;}
    default_action : act_calc_ht_idx();
}

table calc_ht_idx_dup1 {
    actions {act_calc_ht_idx;}
    default_action : act_calc_ht_idx();
}

action act_mark_proc() {
    modify_field(ipv4.protocol, IP_PROCCESSED);
}

table mark_proc {
    actions {act_mark_proc;}
    default_action: act_mark_proc();
}

table mark_proc_dup {
    actions {act_mark_proc;}
    default_action: act_mark_proc();
}
