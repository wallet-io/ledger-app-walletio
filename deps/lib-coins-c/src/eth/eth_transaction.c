#include "eth_transaction.h"
#include "../common/utils.h"
#include "../common/rlp.h"
#include "../common/tx_helper.h"

int eth_tx_init(eth_tx_t* tx, uint8_t* buffer, int buffer_len) {
    if(!tx || !buffer || buffer_len <= 0){
        return 1;
    }
    tx->buffer = buffer;
    tx->buffer_len = buffer_len;
    return eth_tx_update(tx);
}

int eth_tx_update(eth_tx_t* tx) {
    int rlp_length;
    int ret = rlp_get_array_length(tx->buffer, tx->buffer_len, &rlp_length);
    if (ret != 0) {
        return ret;
    }

    return rlp_length > 0 ? 0 : 1;
}

int eth_tx_get_n(eth_tx_t* tx, uint32_t n, uint8_t * out, size_t * out_len, size_t out_cap) {
    return rlp_get_array_item(tx->buffer, tx->buffer_len, n, out, out_len, out_cap);
}

int eth_tx_get_to(eth_tx_t* tx, uint8_t* out, size_t* out_len, size_t out_cap) {
    return eth_tx_get_n(tx, 3, out, out_len, out_cap);
}

int eth_tx_get_value(eth_tx_t* tx, uint8_t * out, size_t * out_len, size_t out_cap) {
    return eth_tx_get_n(tx, 4, out, out_len, out_cap);
}

int eth_tx_get_data(eth_tx_t* tx, uint8_t * out, size_t * out_len, size_t out_cap) {
    return eth_tx_get_n(tx, 5, out, out_len, out_cap);
}

int eth_tx_hash_for_signature(eth_tx_t* tx, hash_func_t hasher, void* param) {
    uint8_t output[32];
    int offset = 0;
    int ret;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_BEGIN);

    for (int j = 0; j < tx->buffer_len; j++) {
        // [output hash]
        if (offset >= 32) {
            HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
        }

        output[offset++] = tx->buffer[j];
    }

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_END);

    return 0;
}