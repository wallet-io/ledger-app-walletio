#ifndef BTC_TRANSACTION_H
#define BTC_TRANSACTION_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "../common/utils.h"

typedef uint32_t sighash_type_t;
#define SIGHASH_ALL 0x00000001
#define SIGHASH_NONE 0x00000002
#define SIGHASH_SINGLE 0x00000003
#define SIGHASH_ANYONECANPAY 0x00000080
#define SIGHASH_BIP143 0x00000040

#define BTC_TX_MASK_NTIME 0x00000001

typedef struct {
    uint8_t * buffer;
    int  buffer_len;
    uint32_t version;
    uint32_t lock_time;
    uint64_t input_len;
    uint32_t input_index;
    uint64_t output_len;
    uint32_t output_index;
    bool has_witnesses;
    uint32_t witnesses_index;
    uint64_t witnesses_len;
    uint32_t mask;
} btc_tx_t;

typedef struct {
    uint8_t hash[32];
    uint32_t index;
    uint32_t sequence;
    uint8_t * script;
    uint64_t script_len;
} btc_tx_input_t;

typedef struct {
    uint64_t value;
    uint8_t * script;
    uint64_t script_len;
} btc_tx_output_t;

int btc_tx_init(btc_tx_t* tx, uint8_t* buffer, int buffer_len);

int btc_tx_update(btc_tx_t* tx);

int btc_tx_get_input(btc_tx_t* tx, int32_t index, btc_tx_input_t* input);

int btc_tx_get_output(btc_tx_t* tx, int32_t index, btc_tx_output_t* output);

int btc_tx_hash_for_signature(btc_tx_t* tx, uint16_t index, sighash_type_t type, uint8_t * prev_out_script, uint16_t script_len, hash_func_t hasher, void* param);

int btc_hash_outputs(btc_tx_t* tx, sighash_type_t type, hash_func_t hasher, void* param);

int btc_hash_inputs(btc_tx_t* tx, sighash_type_t type, hash_func_t hasher, void* param);

int btc_hash_sequence(btc_tx_t* tx, sighash_type_t type, hash_func_t hasher, void* param);

int btc_tx_hash_for_witness_v0(btc_tx_t* tx, uint16_t index, sighash_type_t type, uint8_t * prev_out_script, uint16_t script_len, uint64_t value, uint8_t * hash_inputs, uint8_t * hash_outputs, uint8_t * hash_sequence, hash_func_t hasher, void* param);


#endif