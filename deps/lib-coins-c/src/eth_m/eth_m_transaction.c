#include "eth_m_transaction.h"
#include "../common/rlp.h"
#include "../common/tx_helper.h"
#include "../common/utils.h"

typedef struct {
    uint8_t start_index;
    uint8_t len;
} field_info_t;

static field_info_t field_info[] = {
    {0, 5},    //PREFIX
    {5, 20},   //ADDRESS
    {25, 32},  //VALUE
    {57, 1},   //DATA TODO, the length of this filed is not fixed!
    {58, 32},  //EXPIRETIME
    {90, 32},  //SEQUENCEID
};

static field_info_t token_field_info[] = {
    {0, 5},    //PREFIX
    {5, 20},   //ADDRESS
    {25, 32},  //VALUE
    {57, 20},   //TOKEN ADDRESS
    {77, 32},  //EXPIRETIME
    {109, 32},  //SEQUENCEID
};

int eth_m_tx_init(eth_m_tx_t* tx, uint8_t* buffer, int buffer_len) {
    if (!tx || !buffer) {
        return 1;
    }
    tx->buffer = buffer;
    tx->buffer_len = buffer_len;
    return eth_m_tx_update(tx);
}

int eth_m_tx_update(eth_m_tx_t* tx) {
    return 0;
}

static uint8_t eth_m_tx_get_item(eth_m_tx_t* tx, uint8_t type_index, bool is_token, uint8_t** out) {
    /**
     * check tx,out
     * check out capacity
     * get index and len
     * copy
     * return data len
     */

    if (tx == NULL || out == NULL)
        return 0;
    field_info_t tmp = is_token ? token_field_info[type_index] : field_info[type_index];
    *out = tx->buffer + tmp.start_index;
    return tmp.len;
}

uint8_t eth_m_tx_get_prefix(eth_m_tx_t* tx, uint8_t** out) {
    return eth_m_tx_get_item(tx, 0, false, out);
}

uint8_t eth_m_tx_get_address(eth_m_tx_t* tx, uint8_t** out) {
    return eth_m_tx_get_item(tx, 1, false, out);
}

uint8_t eth_m_tx_get_value(eth_m_tx_t* tx, uint8_t** out) {
    return eth_m_tx_get_item(tx, 2, false, out);
}

// uint8_t eth_m_tx_get_data(eth_m_tx_t* tx, uint8_t** out) {
//     return eth_m_tx_get_item(tx, 3, false, out);
// }

uint8_t eth_m_token_tx_get_prefix(eth_m_tx_t* tx, uint8_t** out) {
    return eth_m_tx_get_item(tx, 0, true, out);
}

uint8_t eth_m_token_tx_get_address(eth_m_tx_t* tx, uint8_t** out) {
    return eth_m_tx_get_item(tx, 1, true, out);
}

uint8_t eth_m_token_tx_get_token_addr(eth_m_tx_t* tx, uint8_t** out) {
    return eth_m_tx_get_item(tx, 3, true, out);
}

uint8_t eth_m_token_tx_get_value(eth_m_tx_t* tx, uint8_t** out ) {
    return eth_m_tx_get_item(tx, 2, true, out);
}

