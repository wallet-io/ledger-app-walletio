#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "coins.h"
#include "networks.h"

#include "btc/btc_transaction.h"
#include "eth/eth_transaction.h"
#include "eos/eos_transaction.h"

#include "os.h"
#include "tx/tx.h"

typedef union {
    btc_tx_t tx_btc;
    eth_tx_t tx_eth;
    eth_tx_t tx_eth_m;
    eos_tx_t tx_eos;
} app_tx_t;

typedef struct {
    union {
        cx_sha256_t sha256;
        cx_sha3_t sha3;
    } hash;
    union {
        uint8_t m32[32];
    } data;
} hash_context_t;

typedef enum {
    SIGN_TX = 1,
    SIGN_MESSAGE
} signing_type_t;

typedef struct app_context {
    uint8_t* buffer;
    const coin_info_t* coin;
    const curr_info_t* curr;
    uint8_t* tx_raw;
    uint32_t tx_raw_len;
    app_tx_t tx;
    hash_context_t hash_context;
    uint8_t* user_data;
    uint32_t user_data_len;
    signing_type_t signing_type;
    bool accept_signing;
    tx_info_page_t page;
} app_context_t;

int app_context_init(app_context_t* context, uint8_t* buffer, signing_type_t type);

int app_context_update(app_context_t* context);

int app_context_reset(app_context_t* context);

int app_context_set_user_data(app_context_t* context, uint8_t* user_data,
                              uint32_t user_data_len);

int app_context_sign_tx(app_context_t* context, uint32_t* path, uint8_t depth,
                        uint32_t index, uint8_t* signature,
                        int signature_capacity, int* signature_length);
