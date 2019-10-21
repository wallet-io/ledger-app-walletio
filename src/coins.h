#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_COIN_TYPE_LENGTH 10
#define MAX_COIN_CATE_LENGTH 10

typedef struct coin_info {
    char symbol[MAX_COIN_TYPE_LENGTH + 1];
    char cate[MAX_COIN_CATE_LENGTH + 1];
    int decimals;
} coin_info_t;

const coin_info_t* get_coin(const char* symbol);

bool is_bip143(const char* symbol);
