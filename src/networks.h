#pragma once

#include <stdint.h>

#define MAX_CURR_TYPE_LENGTH 10

typedef struct curr_info {
    char type[MAX_CURR_TYPE_LENGTH + 1];
    const char* message_prefix;
    uint32_t pub_key_hash;
    uint32_t script_hash;
    union {
        uint32_t i;
        const char* s;
    } chain_id;
} curr_info_t;

const curr_info_t* get_curr_info(const char* curr_type);
