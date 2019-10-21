#include "networks.h"
#include <string.h>

static const curr_info_t all_currencies[] = {
    {"BTC", "Bitcoin Signed Message:\n", 0x01000000, 0x01000005, {0}},
    {"TESTNET3", "Bitcoin Signed Message:\n", 0x0100006f, 0x010000c4, {0}},
    {"ETH", "Ethereum Signed Message:\n", 0, 0, {0x01000001}},
    {"ETC", "Ethereum Signed Message:\n", 0, 0, {0x0100003d}},
    {"ROPSTEN", "Ethereum Signed Message:\n", 0, 0, {0x01000003}},
    {"ETHM", "Ethereum Signed Message:\n", 0, 0, {0}},
    {"ETCM", "Ethereum Signed Message:\n", 0, 0, {0}},
    {"ROPSTENM", "Ethereum Signed Message:\n", 0, 0, {0}},
    {"BCH", "BitcoinCash Signed Message:\n", 0x01000000, 0x01000005, {0}},
    {"BCHSV", "BitcoinCashSV Signed Message:\n", 0x01000000, 0x01000005, {0}},
    {"BTG", "BTG Signed Message:\n", 0x01000026, 0x01000017, {0}},
    {"BCX", "BCX Signed Message:\n", 0x0100004b, 0x0100003f, {0}},
    {"GOD", "GOD Signed Message:\n", 0x01000039, 0x01000027, {0}},
    {"SBTC", "SuperBitcoin Signed Message:\n", 0x01000000, 0x01000005, {0}},
    {"BCD", "BTD Signed Message:\n", 0x01000000, 0x01000005, {0}},
    {"BIFI", "BIFI Signed Message:\n", 0x01000000, 0x01000005, {0}},
    {"LTC", "Litecoin Signed Message:\n", 0x01000030, 0x01000032, {0}},
    {"DOGE", "Dogecoin Signed Message:\n", 0x0100001e, 0x01000016, {0}},
    {"QTUM", "Qtum Signed Message:\n", 0x0100003a, 0x01000032, {0}},
    {"TIPS", "Tipscoin Signed Message:\n", 0x01000021, 0x01000005, {0}},
    {"ZEC", "Zcash Signed Message:\n", 0x02001cb8, 0x02001cbd, {0}},
    {"DASH", "Dash Signed Message:\n", 0x0100004c, 0x01000010, {0}},
    {"XZC", "Zcoin Signed Message:\n", 0x01000052, 0x01000007, {0}},
    {"DCR", "Decred Signed Message:\n", 0x0100073f, 0x0100071a, {0}},
    {"XVG", "Verge Signed Message:\n", 0x0100001e, 0x01000021, {0}},

    {"EOS", "", 0, 0, {0}},
};

const curr_info_t* get_curr_info(const char* curr_type) {
    int length = sizeof(all_currencies) / sizeof(curr_info_t);
    for (int i = 0; i < length; i++) {
        if (!strcmp(curr_type, all_currencies[i].type)) {
            return &all_currencies[i];
        }
    }
    return NULL;
}