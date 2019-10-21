#ifndef EOS_TRANCACTION_H
#define EOS_TRANCACTION_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "../common/utils.h"

typedef struct {
    uint8_t * buffer;
    uint32_t  buffer_len;
    uint32_t tx_raw_index;
    uint32_t tx_raw_len;
    uint32_t action_len;
    uint32_t action_index;
} eos_tx_t;

typedef struct {
    uint64_t account;
    uint64_t name;
    uint32_t authorization_len;
    uint32_t authorization_index;
    uint8_t * data;
    uint32_t data_len;
} eos_tx_action_t;

typedef struct {
    uint64_t from;
    uint64_t to;
    int64_t amount;
    uint64_t symbol;
    uint8_t * memo;
    uint32_t memo_len;
} eos_tx_action_transfer_t;

typedef struct {
    uint16_t type;
    uint8_t * data;
    uint32_t data_len;
} eos_tx_extension_t;

int eos_tx_init(eos_tx_t* tx, uint8_t* buffer, int buffer_len);

int eos_tx_update(eos_tx_t* tx);

int eos_tx_get_action(eos_tx_t* tx, uint32_t index, eos_tx_action_t* action);

int eos_decode_name(uint64_t name, uint8_t * out, uint32_t * out_len);

int eos_tx_hash_for_signature(eos_tx_t* tx, hash_func_t hasher, void* param);

#endif