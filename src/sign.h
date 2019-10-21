#pragma once

#include <os.h>
#include <os_io_seproxyhal.h>

int sign_secp256k1(const uint8_t *message, unsigned int message_length,
                   uint8_t *signature, unsigned int signature_capacity,
                   unsigned int *signature_length,
                   cx_ecfp_private_key_t *private_key);

int sign_secp256k1_canonical(const uint8_t *message,
                             unsigned int message_length, uint8_t *signature,
                             unsigned int signature_capacity,
                             unsigned int *signature_length,
                             cx_ecfp_private_key_t *private_key);
