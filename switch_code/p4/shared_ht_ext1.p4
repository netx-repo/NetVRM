#define REG_LEN 65536
#define REG_LEN_LOG 16
// REG_LEN = (1<<REG_LEN_LOG)

register ht_stg5 {
    width: 64;
    instance_count: REG_LEN;
}

register ht_stg6 {
    width: 64;
    instance_count: REG_LEN;
}

register ht_stg7 {
    width: 64;
    instance_count: REG_LEN;
}

register ht_stg8 {
    width: 64;
    instance_count: REG_LEN;
}

blackbox stateful_alu salu_evict_ht_stg5 {
    reg: ht_stg5;

    update_lo_1_value: ht_md.key_hdr;
    output_value: register_lo;
    output_dst: ht_md.stg5_lo;
}

action act_evict_ht_stg5() {
    salu_evict_ht_stg5.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 7
table evict_ht_stg5 {
    actions {act_evict_ht_stg5;}
    default_action: act_evict_ht_stg5();
}

@pragma stage 7
table evict_ht_stg5_dup {
    actions {act_evict_ht_stg5;}
    default_action: act_evict_ht_stg5();
}

// for sketch
blackbox stateful_alu salu_incr_ht_stg5 {
    reg: ht_stg5;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg5() {
    salu_incr_ht_stg5.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 7
table incr_ht_stg5 {
    actions {act_incr_ht_stg5;}
    default_action: act_incr_ht_stg5();
}

blackbox stateful_alu salu_cmp_ht_stg5 {
    reg: ht_stg5;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ht_md.key_hdr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg5_lo;
}

action act_cmp_ht_stg5() {
    salu_cmp_ht_stg5.execute_stateful_alu(ht_md.app1_idx);
}

table cmp_ht_stg5 {
    actions {act_cmp_ht_stg5;}
    default_action: act_cmp_ht_stg5();
}

blackbox stateful_alu salu_incr_ht_stg6 {
    reg: ht_stg6;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg6() {
    salu_incr_ht_stg6.execute_stateful_alu(ht_md.app1_idx);
}

table incr_ht_stg6 {
    actions {act_incr_ht_stg6;}
    default_action: act_incr_ht_stg6();
}

// for sketch
action act_incr_ht_stg6_hash2() {
    salu_incr_ht_stg6.execute_stateful_alu(ht_md.app1_idx_hash2);
}

table incr_ht_stg6_hash2 {
    actions {act_incr_ht_stg6_hash2;}
    default_action: act_incr_ht_stg6_hash2();
}

blackbox stateful_alu salu_reset_ht_stg6 {
    reg: ht_stg6;
    update_lo_1_value: 1;
    output_value : register_lo;
    output_dst : ht_md.stg6_lo;
}

action act_reset_ht_stg6() {
    salu_reset_ht_stg6.execute_stateful_alu(ht_md.app1_idx);
}

table reset_ht_stg6 {
    actions {act_reset_ht_stg6;}
    default_action: act_reset_ht_stg6();
}

// for superspreader
blackbox stateful_alu salu_evict_ht_stg6 {
    reg: ht_stg6;

    update_lo_1_value: ipv4.dstAddr;
    output_value: register_lo;
    output_dst: ht_md.stg6_lo;
}

action act_evict_ht_stg6() {
    salu_evict_ht_stg6.execute_stateful_alu(ht_md.app1_idx);
}

table evict_ht_stg6 {
    actions {act_evict_ht_stg6;}
    default_action: act_evict_ht_stg6();
}

blackbox stateful_alu salu_cmp_ht_stg6 {
    reg: ht_stg6;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ipv4.dstAddr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg6_lo;
}

action act_cmp_ht_stg6() {
    salu_cmp_ht_stg6.execute_stateful_alu(ht_md.app1_idx);
}

table cmp_ht_stg6 {
    actions {act_cmp_ht_stg6;}
    default_action: act_cmp_ht_stg6();
}

/*****
hash table stage 7 and stage 8
*****/
blackbox stateful_alu salu_evict_ht_stg7 {
    reg: ht_stg7;

    update_lo_1_value: ht_md.key_hdr;

    output_value: register_lo;
    output_dst: ht_md.stg7_lo;
}

action act_evict_ht_stg7() {
    salu_evict_ht_stg7.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 9
table evict_ht_stg7 {
    actions {act_evict_ht_stg7;}
    default_action: act_evict_ht_stg7();
}

@pragma stage 9
table evict_ht_stg7_dup {
    actions {act_evict_ht_stg7;}
    default_action: act_evict_ht_stg7();
}

// for sketch
blackbox stateful_alu salu_incr_ht_stg7 {
    reg: ht_stg7;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg7() {
    salu_incr_ht_stg7.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 9
table incr_ht_stg7 {
    actions {act_incr_ht_stg7;}
    default_action: act_incr_ht_stg7();
}

blackbox stateful_alu salu_cmp_ht_stg7 {
    reg: ht_stg7;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ht_md.key_hdr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg7_lo;
}

action act_cmp_ht_stg7() {
    salu_cmp_ht_stg7.execute_stateful_alu(ht_md.app1_idx);
}

table cmp_ht_stg7 {
    actions {act_cmp_ht_stg7;}
    default_action: act_cmp_ht_stg7();
}

blackbox stateful_alu salu_incr_ht_stg8 {
    reg: ht_stg8;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg8() {
    salu_incr_ht_stg8.execute_stateful_alu(ht_md.app1_idx);
}

table incr_ht_stg8 {
    actions {act_incr_ht_stg8;}
    default_action: act_incr_ht_stg8();
}

// for sketch
action act_incr_ht_stg8_hash2() {
    salu_incr_ht_stg8.execute_stateful_alu(ht_md.app1_idx_hash2);
}

table incr_ht_stg8_hash2 {
    actions {act_incr_ht_stg8_hash2;}
    default_action: act_incr_ht_stg8_hash2();
}

blackbox stateful_alu salu_reset_ht_stg8 {
    reg: ht_stg8;
    update_lo_1_value: 1;
    output_value : register_lo;
    output_dst : ht_md.stg8_lo;
}

action act_reset_ht_stg8() {
    salu_reset_ht_stg8.execute_stateful_alu(ht_md.app1_idx);
}

table reset_ht_stg8 {
    actions {act_reset_ht_stg8;}
    default_action: act_reset_ht_stg8();
}

// for superspreader
blackbox stateful_alu salu_evict_ht_stg8 {
    reg: ht_stg8;

    update_lo_1_value: ipv4.dstAddr;
    output_value: register_lo;
    output_dst: ht_md.stg8_lo;
}

action act_evict_ht_stg8() {
    salu_evict_ht_stg8.execute_stateful_alu(ht_md.app1_idx);
}

table evict_ht_stg8 {
    actions {act_evict_ht_stg8;}
    default_action: act_evict_ht_stg8();
}

blackbox stateful_alu salu_cmp_ht_stg8 {
    reg: ht_stg8;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ipv4.dstAddr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg8_lo;
}

action act_cmp_ht_stg8() {
    salu_cmp_ht_stg8.execute_stateful_alu(ht_md.app1_idx);
}

table cmp_ht_stg8 {
    actions {act_cmp_ht_stg8;}
    default_action: act_cmp_ht_stg8();
}

// for netcache
blackbox stateful_alu salu_read_ht_stg5 {
    reg: ht_stg5;
    output_value: register_lo;
    output_dst: ht_md.stg5_lo;
}

action act_read_ht_stg5() {
    salu_read_ht_stg5.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 7
table read_ht_stg5 {
    actions {act_read_ht_stg5;}
    default_action: act_read_ht_stg5();
}

blackbox stateful_alu salu_read_ht_stg6 {
    reg: ht_stg6;
    output_value: register_lo;
    output_dst: ht_md.stg6_lo;
}

action act_read_ht_stg6() {
    salu_read_ht_stg6.execute_stateful_alu(ht_md.app1_idx);
}

table read_ht_stg6 {
    actions {act_read_ht_stg6;}
    default_action: act_read_ht_stg6();
}

blackbox stateful_alu salu_read_ht_stg7 {
    reg: ht_stg7;
    output_value: register_lo;
    output_dst: ht_md.stg7_lo;
}

action act_read_ht_stg7() {
    salu_read_ht_stg7.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 9
table read_ht_stg7 {
    actions {act_read_ht_stg7;}
    default_action: act_read_ht_stg7();
}

blackbox stateful_alu salu_read_ht_stg8 {
    reg: ht_stg8;
    output_value: register_lo;
    output_dst: ht_md.stg8_lo;
}

action act_read_ht_stg8() {
    salu_read_ht_stg8.execute_stateful_alu(ht_md.app1_idx);
}

table read_ht_stg8 {
    actions {act_read_ht_stg8;}
    default_action: act_read_ht_stg8();
}
