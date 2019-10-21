#pragma once

#include <stdbool.h>
#include <stdint.h>

struct app_context;
typedef struct app_context app_context_t;

typedef struct tx_info_page {
    uint8_t amount_str[32];
    uint8_t title_str[32];
    uint8_t eth_token_addr_str[45];
    uint8_t to[45];
} tx_info_page_t;

int tx_init(app_context_t* context);

int tx_sign(app_context_t* context, uint32_t* path, uint8_t depth,
                        uint32_t index, uint8_t* signature,
                        int signature_capacity, int* signature_length);

int get_tx_info_page_size(app_context_t* context);

int get_tx_info_page(app_context_t* context, uint32_t index, tx_info_page_t* page);