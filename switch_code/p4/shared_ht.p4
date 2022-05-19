#define REG_LEN 65536
#define REG_LEN_LOG 16
// REG_LEN = (1<<REG_LEN_LOG)

register ht_stg1 {
    width: 64;
    instance_count: REG_LEN;
}

register ht_stg2 {
    width: 64;
    instance_count: REG_LEN;
}

register ht_stg3 {
    width: 64;
    instance_count: REG_LEN;
}

register ht_stg4 {
    width: 64;
    instance_count: REG_LEN;
}

header_type ht_md_t {
    fields {
        stg1_lo: 32;
        stg2_lo: 32;
        stg3_lo: 32;
        stg4_lo: 32;
        stg5_lo: 32;
        stg6_lo: 32;
        stg7_lo: 32;
        stg8_lo: 32;
        app1_idx: 32;
        app1_idx_hash2: 32;
        app2_idx: 32;
        key_hdr: 32;
        sport: 32;
        dport: 32;
        port_pair: 32;
    }
}

metadata ht_md_t ht_md;

action act_set_srcip_key() {
    modify_field(ht_md.key_hdr, ipv4.srcAddr);
}

table set_srcip_key {
    actions {act_set_srcip_key;}
    default_action: act_set_srcip_key();
}

table set_srcip_key_dup {
    actions {act_set_srcip_key;}
    default_action: act_set_srcip_key();
}

table set_srcip_key_dup1 {
    actions {act_set_srcip_key;}
    default_action: act_set_srcip_key();
}

action act_set_dstip_key() {
    modify_field(ht_md.key_hdr, ipv4.dstAddr);
}

table set_dstip_key {
    actions {act_set_dstip_key;}
    default_action: act_set_dstip_key();
}

blackbox stateful_alu salu_evict_ht_stg1 {
    reg: ht_stg1;

    update_lo_1_value: ht_md.key_hdr;
    output_value: register_lo;
    output_dst: ht_md.stg1_lo;
}

action act_evict_ht_stg1() {
    salu_evict_ht_stg1.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 3
table evict_ht_stg1 {
    actions {act_evict_ht_stg1;}
    default_action: act_evict_ht_stg1();
}

@pragma stage 3
table evict_ht_stg1_dup {
    actions {act_evict_ht_stg1;}
    default_action: act_evict_ht_stg1();
}

// for sketch
blackbox stateful_alu salu_incr_ht_stg1 {
    reg: ht_stg1;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg1() {
    salu_incr_ht_stg1.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 3
table incr_ht_stg1 {
    actions {act_incr_ht_stg1;}
    default_action: act_incr_ht_stg1();
}

blackbox stateful_alu salu_cmp_ht_stg1 {
    reg: ht_stg1;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ht_md.key_hdr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg1_lo;
}

action act_cmp_ht_stg1() {
    salu_cmp_ht_stg1.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 3
table cmp_ht_stg1 {
    actions {act_cmp_ht_stg1;}
    default_action: act_cmp_ht_stg1();
}

blackbox stateful_alu salu_incr_ht_stg2 {
    reg: ht_stg2;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg2() {
    salu_incr_ht_stg2.execute_stateful_alu(ht_md.app1_idx);
}

table incr_ht_stg2 {
    actions {act_incr_ht_stg2;}
    default_action: act_incr_ht_stg2();
}

// for sketch
action act_incr_ht_stg2_hash2() {
    salu_incr_ht_stg2.execute_stateful_alu(ht_md.app1_idx_hash2);
}

table incr_ht_stg2_hash2 {
    actions {act_incr_ht_stg2_hash2;}
    default_action: act_incr_ht_stg2_hash2();
}

blackbox stateful_alu salu_reset_ht_stg2 {
    reg: ht_stg2;
    update_lo_1_value: 1;
    output_value : register_lo;
    output_dst : ht_md.stg2_lo;
}

action act_reset_ht_stg2() {
    salu_reset_ht_stg2.execute_stateful_alu(ht_md.app1_idx);
}

table reset_ht_stg2 {
    actions {act_reset_ht_stg2;}
    default_action: act_reset_ht_stg2();
}

// for superspreader
blackbox stateful_alu salu_evict_ht_stg2 {
    reg: ht_stg2;

    update_lo_1_value: ipv4.dstAddr;
    output_value: register_lo;
    output_dst: ht_md.stg2_lo;
}

action act_evict_ht_stg2() {
    salu_evict_ht_stg2.execute_stateful_alu(ht_md.app1_idx);
}

table evict_ht_stg2 {
    actions {act_evict_ht_stg2;}
    default_action: act_evict_ht_stg2();
}

blackbox stateful_alu salu_cmp_ht_stg2 {
    reg: ht_stg2;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ipv4.dstAddr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg2_lo;
}

action act_cmp_ht_stg2() {
    salu_cmp_ht_stg2.execute_stateful_alu(ht_md.app1_idx);
}

table cmp_ht_stg2 {
    actions {act_cmp_ht_stg2;}
    default_action: act_cmp_ht_stg2();
}

/*****
hash table stage 3 and stage 4
*****/
blackbox stateful_alu salu_evict_ht_stg3 {
    reg: ht_stg3;
#ifdef FOURTUPLE
    update_lo_1_value: ht_md.port_pair;
#else
    update_lo_1_value: ht_md.key_hdr;
#endif
    output_value: register_lo;
    output_dst: ht_md.stg3_lo;
}

action act_evict_ht_stg3() {
    salu_evict_ht_stg3.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 5
table evict_ht_stg3 {
    actions {act_evict_ht_stg3;}
    default_action: act_evict_ht_stg3();
}

@pragma stage 5
table evict_ht_stg3_dup {
    actions {act_evict_ht_stg3;}
    default_action: act_evict_ht_stg3();
}

// for sketch
blackbox stateful_alu salu_incr_ht_stg3 {
    reg: ht_stg3;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg3() {
    salu_incr_ht_stg3.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 5
table incr_ht_stg3 {
    actions {act_incr_ht_stg3;}
    default_action: act_incr_ht_stg3();
}


blackbox stateful_alu salu_cmp_ht_stg3 {
    reg: ht_stg3;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ht_md.key_hdr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg3_lo;
}

action act_cmp_ht_stg3() {
    salu_cmp_ht_stg3.execute_stateful_alu(ht_md.app1_idx);
}

table cmp_ht_stg3 {
    actions {act_cmp_ht_stg3;}
    default_action: act_cmp_ht_stg3();
}

blackbox stateful_alu salu_incr_ht_stg4 {
    reg: ht_stg4;
    update_lo_1_value: register_lo+1;
}

action act_incr_ht_stg4() {
    salu_incr_ht_stg4.execute_stateful_alu(ht_md.app1_idx);
}

table incr_ht_stg4 {
    actions {act_incr_ht_stg4;}
    default_action: act_incr_ht_stg4();
}

// for sketch
action act_incr_ht_stg4_hash2() {
    salu_incr_ht_stg4.execute_stateful_alu(ht_md.app1_idx_hash2);
}

table incr_ht_stg4_hash2 {
    actions {act_incr_ht_stg4_hash2;}
    default_action: act_incr_ht_stg4_hash2();
}

blackbox stateful_alu salu_reset_ht_stg4 {
    reg: ht_stg4;
    update_lo_1_value: 1;
    output_value : register_lo;
    output_dst : ht_md.stg4_lo;
}

action act_reset_ht_stg4() {
    salu_reset_ht_stg4.execute_stateful_alu(ht_md.app1_idx);
}

table reset_ht_stg4 {
    actions {act_reset_ht_stg4;}
    default_action: act_reset_ht_stg4();
}

// for superspreader
blackbox stateful_alu salu_evict_ht_stg4 {
    reg: ht_stg4;

    update_lo_1_value: ipv4.dstAddr;
    output_value: register_lo;
    output_dst: ht_md.stg4_lo;
}

action act_evict_ht_stg4() {
    salu_evict_ht_stg4.execute_stateful_alu(ht_md.app1_idx);
}

table evict_ht_stg4 {
    actions {act_evict_ht_stg4;}
    default_action: act_evict_ht_stg4();
}

blackbox stateful_alu salu_cmp_ht_stg4 {
    reg: ht_stg4;

    condition_lo: register_lo != 0;
    condition_hi: register_lo != ipv4.dstAddr;
    update_lo_1_value: ht_md.key_hdr;

    output_predicate : condition_lo and condition_hi;
    output_value: combined_predicate;
    output_dst: ht_md.stg4_lo;
}

action act_cmp_ht_stg4() {
    salu_cmp_ht_stg4.execute_stateful_alu(ht_md.app1_idx);
}

table cmp_ht_stg4 {
    actions {act_cmp_ht_stg4;}
    default_action: act_cmp_ht_stg4();
}

// for netcache
blackbox stateful_alu salu_read_ht_stg1 {
    reg: ht_stg1;
    output_value: register_lo;
    output_dst: ht_md.stg1_lo;
}

action act_read_ht_stg1() {
    salu_read_ht_stg1.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 3
table read_ht_stg1 {
    actions {act_read_ht_stg1;}
    default_action: act_read_ht_stg1();
}

blackbox stateful_alu salu_read_ht_stg2 {
    reg: ht_stg2;
    output_value: register_lo;
    output_dst: ht_md.stg2_lo;
}

action act_read_ht_stg2() {
    salu_read_ht_stg2.execute_stateful_alu(ht_md.app1_idx);
}

table read_ht_stg2 {
    actions {act_read_ht_stg2;}
    default_action: act_read_ht_stg2();
}

blackbox stateful_alu salu_read_ht_stg3 {
    reg: ht_stg3;
    output_value: register_lo;
    output_dst: ht_md.stg3_lo;
}

action act_read_ht_stg3() {
    salu_read_ht_stg3.execute_stateful_alu(ht_md.app1_idx);
}

@pragma stage 5
table read_ht_stg3 {
    actions {act_read_ht_stg3;}
    default_action: act_read_ht_stg3();
}

blackbox stateful_alu salu_read_ht_stg4 {
    reg: ht_stg4;
    output_value: register_lo;
    output_dst: ht_md.stg4_lo;
}

action act_read_ht_stg4() {
    salu_read_ht_stg4.execute_stateful_alu(ht_md.app1_idx);
}

table read_ht_stg4 {
    actions {act_read_ht_stg4;}
    default_action: act_read_ht_stg4();
}
