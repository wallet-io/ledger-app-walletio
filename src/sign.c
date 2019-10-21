#include "sign.h"

int sign_secp256k1(const uint8_t *message, unsigned int message_length,
                   uint8_t *signature, unsigned int signature_capacity,
                   unsigned int *signature_length,
                   cx_ecfp_private_key_t *private_key) {
    uint8_t message_digest[CX_SHA256_SIZE];
    os_memset(&message_digest, 0, sizeof(message_digest));
    os_memmove(
        message_digest, message,
        message_length > CX_SHA256_SIZE ? CX_SHA256_SIZE : message_length);

    uint8_t temp_signature[100];
    uint8_t temp_signature_length;
    uint8_t r_length, s_length, r_offset, s_offset;
    unsigned int info = 0;
    temp_signature_length = cx_ecdsa_sign(
        private_key, CX_RND_RFC6979 | CX_LAST, CX_SHA256, message_digest,
        CX_SHA256_SIZE, temp_signature, sizeof(temp_signature), &info);

    signature[0] = 27;
    if (info & CX_ECCINFO_PARITY_ODD) {
        signature[0]++;
    }
    if (info & CX_ECCINFO_xGTn) {
        signature[0] += 2;
    }

    r_length = temp_signature[3];
    s_length = temp_signature[4 + r_length + 1];
    r_offset = (r_length == 33 ? 1 : 0);
    s_offset = (s_length == 33 ? 1 : 0);
    if (32 - r_length > 0) os_memset(signature + 1, 0, 32 - r_length);
    os_memmove(signature + 1 + (32 > r_length ? 32 - r_length : 0),
               temp_signature + 4 + r_offset, (32 > r_length ? r_length : 32));
    if (32 - s_length > 0) os_memset(signature + 1 + 32, 0, 32 - s_length);
    os_memmove(signature + 1 + 32 + (32 > s_length ? 32 - s_length : 0),
               temp_signature + 4 + r_length + 2 + s_offset,
               (32 > s_length ? s_length : 32));
    *signature_length = 65;

    return 0;
}

uint8_t const SECP256K1_N[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
                               0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b,
                               0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41};

int sign_secp256k1_canonical(const uint8_t *message,
                             unsigned int message_length, uint8_t *signature,
                             unsigned int signature_capacity,
                             unsigned int *signature_length,
                             cx_ecfp_private_key_t *private_key) {
    uint8_t message_digest[CX_SHA256_SIZE];
    os_memset(&message_digest, 0, sizeof(message_digest));
    os_memmove(
        message_digest, message,
        message_length > CX_SHA256_SIZE ? CX_SHA256_SIZE : message_length);

    uint8_t temp_signature[100];
    uint8_t temp_signature_length;
    uint8_t V[33];
    uint8_t K[32];
    int tries = 0;

    // Loop until a candidate matching the canonical signature is found
    for (;;) {
        if (tries == 0) {
            rng_rfc6979(temp_signature, message_digest, private_key->d, private_key->d_len, SECP256K1_N, 32, V, K);
        } else {
            rng_rfc6979(temp_signature, message_digest, NULL, 0, SECP256K1_N, 32, V, K);
        }
        uint32_t info;
        temp_signature_length = cx_ecdsa_sign(private_key, CX_NO_CANONICAL | CX_RND_PROVIDED | CX_LAST, CX_SHA256, message_digest, 32, temp_signature, sizeof(temp_signature), &info);
        if ((info & CX_ECCINFO_PARITY_ODD) != 0) {
            temp_signature[0] |= 0x01;
        }
        signature[0] = 27 + 4 + (temp_signature[0] & 0x01);
        ecdsa_der_to_sig(temp_signature, signature + 1);
        if (check_canonical(signature + 1)) {
            *signature_length = 65;
            break;
        } else {
            tries++;
        }

        PRINTF("sign_secp256k1_canonical tries:%d\n",tries);
    }

    return 0;
}
