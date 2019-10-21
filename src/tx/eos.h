#pragma once

#include <stdbool.h>
#include <stdint.h>

struct app_context;
typedef struct app_context app_context_t;

struct tx_info_page;
typedef struct tx_info_page tx_info_page_t;

int eos_init_tx(app_context_t* context);

int eos_sign_tx(app_context_t* context, uint32_t* path, uint8_t depth,
                        uint32_t index, uint8_t* signature,
                        int signature_capacity, int* signature_length);

int eos_get_tx_info_page_size(app_context_t* context);

int eos_get_tx_info_page(app_context_t* context, uint32_t index, tx_info_page_t* page);
int eos_sign_message(app_context_t* context, uint32_t* path, uint8_t depth, uint8_t* signature,int signature_capacity, int* signature_length);