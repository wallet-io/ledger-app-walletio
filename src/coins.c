#pragma once

#include "coins.h"
#include <string.h>

static const coin_info_t all_coins[] = {
    {"BTC", "BTC", 8},
    {"TESTNET3", "BTC", 8},
    {"BCH", "BTC", 8},
    {"BCHSV", "BTC", 8},
    {"BTG", "BTC", 8},
    {"BCX", "BTC", 4},
    {"GOD", "BTC", 8},
    {"SBTC", "BTC", 8},
    {"BCD", "BTC", 7},
    {"BIFI", "BTC", 5},
    {"LTC", "BTC", 8},
    {"DOGE", "BTC", 8},
    {"QTUM", "BTC", 8},
    {"TIPS", "BTC", 8},
    {"ZEC", "BTC", 8},
    {"DASH", "BTC", 8},
    {"XZC", "BTC", 8},
    {"DCR", "BTC", 8},
    {"XVG", "BTC", 6},

    {"ETH", "ETH", 18},
    {"ETC", "ETH", 18},
    {"ROPSTEN", "ETH", 18},

    {"ETHM", "ETHM", 18},
    {"ETCM", "ETHM", 18},
    {"ROPSTENM", "ETHM", 18},

    {"EOS", "EOS", 4},
};

const coin_info_t* get_coin(const char* symbol) {
    int length = sizeof(all_coins) / sizeof(coin_info_t);
    for (int i = 0; i < length; i++) {
        if (!strcmp(symbol, all_coins[i].symbol)) {
            return &all_coins[i];
        }
    }
    return NULL;
}

bool is_bip143(const char* symbol) {
    return !strcmp(symbol, "BCH") || !strcmp(symbol, "BCHSV") || !strcmp(symbol, "BTG") || !strcmp(symbol, "BCX") || !strcmp(symbol, "GOD");
}
