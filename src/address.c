#include "address.h"
#include "common/base58.h"

#include <stdbool.h>
#include "os.h"

bool is_pub_key_hash(uint8_t* script, uint32_t script_len) {
    return script_len == 25 && script[0] == 0x76       // OP_DUP
           && script[1] == 0xa9                        // OP_HASH160
           && script[2] == 0x14 && script[23] == 0x88  // OP_EQUALVERIFY
           && script[24] == 0xac;                      // OP_CHECKSIG
}

bool is_script_hash(uint8_t* script, uint32_t script_len) {
    return script_len == 23 && script[0] == 0xa9        // OP_HASH160
           && script[1] == 0x14 && script[22] == 0x87;  // OP_EQUALVERIFY
}

int to_base58_check(uint8_t* data, uint32_t data_len, uint32_t version,
                    uint8_t* address, uint32_t* address_len) {
    // PRINTF("to_base58_check: %.*h %d\n", data_len, data, version);
    cx_sha256_t hash;
    uint8_t payload[32] = {0};
    // PRINTF("to_base58_check payload 1: %.*h\n", sizeof(payload), payload);
    uint8_t version_len = (version & 0xff000000) >> 24;
    version_len = version_len > 0 ? version_len : 1;
    for (int i = 0; i < version_len; i++) {
        payload[version_len - 1 - i] =
            (uint8_t)(0x000000ff & (version >> (i * 8)));
    }
    // PRINTF("to_base58_check payload 2: %.*h\n", sizeof(payload), payload);
    os_memmove(payload + version_len, data, data_len);
    // PRINTF("to_base58_check payload 3: %.*h\n", sizeof(payload), payload);

    cx_sha256_init(&hash);
    cx_hash(&hash.header, CX_LAST, payload, version_len + data_len, payload,
            sizeof(payload));
    // PRINTF("to_base58_check payload 4: %.*h\n", sizeof(payload), payload);
    cx_sha256_init(&hash);
    cx_hash(&hash.header, CX_LAST, payload, sizeof(payload), payload,
            sizeof(payload));
    // PRINTF("to_base58_check payload 5: %.*h\n", sizeof(payload), payload);

    os_memmove(payload + version_len + data_len, payload, 4);
    // PRINTF("to_base58_check payload 6: %.*h\n", sizeof(payload), payload);
    for (int i = 0; i < version_len; i++) {
        payload[version_len - 1 - i] =
            (uint8_t)(0x000000ff & (version >> (i * 8)));
    }
    // PRINTF("to_base58_check payload 7: %.*h\n", sizeof(payload), payload);
    os_memmove(payload + version_len, data, data_len);
    // PRINTF("to_base58_check payload 8: %.*h\n", sizeof(payload), payload);
    int ret = encode_base58(payload, version_len + data_len + 4, address, 64);
    if (ret < 0) {
        *address_len = 0;
        return 1;
    }
    *address_len = ret;
    return 0;
}

int gen_btc_address(uint8_t* script, uint32_t script_len, uint32_t pub_key_hash,
                    uint32_t script_hash, uint8_t* address,
                    uint32_t* address_len) {
    // PRINTF("gen_btc_address: %.*h %d %d\n", script_len, script, pub_key_hash,
    // script_hash);
    int ret = 0;
    if (is_pub_key_hash(script, script_len)) {
        ret =
            to_base58_check(script + 3, 20, pub_key_hash, address, address_len);
    } else if (is_script_hash(script, script_len)) {
        ret =
            to_base58_check(script + 2, 20, script_hash, address, address_len);
    } else {
        ret = 1;
    }
    return ret;
}