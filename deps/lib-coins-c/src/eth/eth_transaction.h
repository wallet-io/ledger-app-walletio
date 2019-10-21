#ifndef ETH_TRANCACTION_H
#define ETH_TRANCACTION_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../common/rlp.h"
#include "../common/utils.h"

typedef struct {
    uint8_t * buffer;
    int  buffer_len;
} eth_tx_t;

int eth_tx_init(eth_tx_t* tx, uint8_t* buffer, int buffer_len);

int eth_tx_update(eth_tx_t* tx);

int eth_tx_get_n(eth_tx_t* tx, uint32_t n, uint8_t * out, size_t * out_len, size_t out_cap);

int eth_tx_get_to(eth_tx_t* tx, uint8_t * out, size_t * out_len, size_t out_cap);

int eth_tx_get_value(eth_tx_t* tx, uint8_t * out, size_t * out_len, size_t out_cap);

int eth_tx_get_data(eth_tx_t* tx, uint8_t * out, size_t * out_len, size_t out_cap);

int eth_tx_hash_for_signature(eth_tx_t* tx, hash_func_t hasher, void* param);

#endif