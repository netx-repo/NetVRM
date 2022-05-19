// Batches a series of API operations.
#define BATCHED(code) \
    p4_pd_begin_batch(sess_hdl); \
    code \
    p4_pd_end_batch(sess_hdl, true);

