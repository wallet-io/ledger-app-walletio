#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum { SECP256K1 = 0, SECP256R1, ED25519 } sig_type_t;

int get_pub_key(uint8_t *public_key_data, uint8_t *chain_code, sig_type_t curve,
                int *bip32_path, int bip32_depth);

int sign(const uint8_t *message, int message_length, uint8_t *signature,
         int signature_capacity, int *signature_length, sig_type_t curve,
         int *bip32_path, int bip32_depth, bool canonical);