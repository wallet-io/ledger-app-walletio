#include "crypto.h"
#include <os.h>
#include <os_io_seproxyhal.h>
#include "sign.h"

void generate_pair(cx_ecfp_public_key_t *public_key,
                   cx_ecfp_private_key_t *private_key,
                   const uint8_t private_key_data[32], const cx_curve_t curve) {
    cx_ecfp_init_private_key(curve, private_key_data, 32, private_key);
    cx_ecfp_init_public_key(curve, NULL, 0, public_key);
    cx_ecfp_generate_pair(curve, public_key, private_key, 1);
}

int derive_key_pair(cx_ecfp_public_key_t *public_key,
                    cx_ecfp_private_key_t *private_key, uint8_t *chain_code,
                    sig_type_t curve, int *bip32_path, int bip32_depth) {
    cx_curve_t cx_curve;
    if (curve == SECP256K1) {
        cx_curve = CX_CURVE_256K1;
    } else if (curve == ED25519) {
        cx_curve = CX_CURVE_Ed25519;
    } else {
        return 1;
    }

    uint8_t private_key_data[32];
    os_perso_derive_node_bip32(cx_curve, bip32_path, bip32_depth,
                               private_key_data, chain_code);

    generate_pair(public_key, private_key, private_key_data, cx_curve);
    return 0;
}

int get_pub_key(uint8_t *public_key_data, uint8_t *chain_code, sig_type_t curve,
                int *bip32_path, int bip32_depth) {
    cx_ecfp_public_key_t public_key;
    cx_ecfp_private_key_t private_key;

    int ret = derive_key_pair(&public_key, &private_key, chain_code, curve,
                              bip32_path, bip32_depth);
    if (ret != 0) {
        return ret;
    }

    os_memmove(public_key_data, public_key.W, 65);

    return 0;
}

int sign(const uint8_t *message, int message_length, uint8_t *signature, int signature_capacity, int *signature_length, sig_type_t curve, int *bip32_path, int bip32_depth, bool canonical) {
    cx_ecfp_public_key_t public_key;
    cx_ecfp_private_key_t private_key;

    int ret = derive_key_pair(&public_key, &private_key, NULL, curve,
                              bip32_path, bip32_depth);
    if (ret != 0) {
        return ret;
    }

    if (curve == SECP256K1) {
        if (canonical) {
            return sign_secp256k1_canonical(message, message_length, signature,
                                            signature_capacity,
                                            signature_length, &private_key);
        } else {
            return sign_secp256k1(message, message_length, signature,
                                  signature_capacity, signature_length,
                                  &private_key);
        }
    }
    
    return 1;
}
