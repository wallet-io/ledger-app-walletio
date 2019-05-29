/*******************************************************************************
*   (c) 2016 Ledger
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/
#include "signature.h"

#include "cx.h"
#include "apdu_codes.h"

#include "lib/eos_utils.h"

uint8_t const SECP256K1_N[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                               0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
                               0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b,
                               0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41};

void keys(cx_ecfp_public_key_t *publicKey, cx_ecfp_private_key_t *privateKey, const uint8_t privateKeyData[32], const cx_curve_t curve) {
    cx_ecfp_init_private_key(curve, privateKeyData, 32, privateKey);
    cx_ecfp_init_public_key(curve, NULL, 0, publicKey);
    cx_ecfp_generate_pair(curve, publicKey, privateKey, 1);
}

int sign_secp256k1(const uint8_t *message,
                   unsigned int message_length,
                   uint8_t *signature,
                   unsigned int signature_capacity,
                   unsigned int *signature_length,
                   cx_ecfp_private_key_t *privateKey) {
    uint8_t message_digest[CX_SHA256_SIZE];
    unsigned int i;
    for (i = 0; i < CX_SHA256_SIZE; i++) {
        if (i < message_length) {
            message_digest[i] = message[i];
        }
        else {
            message_digest[i] = 0;
        }
    }
    
    cx_ecfp_public_key_t publicKey;
    cx_ecdsa_init_public_key(CX_CURVE_256K1, NULL, 0, &publicKey);
    cx_ecfp_generate_pair(CX_CURVE_256K1, &publicKey, privateKey, 1);

    uint8_t tempSignature[100];
    uint8_t signatureLength;
    uint8_t rLength, sLength, rOffset, sOffset;
    unsigned int info = 0;
    signatureLength = cx_ecdsa_sign(
        privateKey,
        CX_RND_RFC6979 | CX_LAST,
        CX_SHA256,
        message_digest,
        CX_SHA256_SIZE,
        tempSignature,
        sizeof(tempSignature),
        &info);
    os_memset(&privateKey, 0, sizeof(privateKey));
    signature[0] = 27;
    if (info & CX_ECCINFO_PARITY_ODD) {
      signature[0]++;
    }
    if (info & CX_ECCINFO_xGTn) {
      signature[0] += 2;
    }

    rLength = tempSignature[3];
    sLength = tempSignature[4 + rLength + 1];
    rOffset = (rLength == 33 ? 1 : 0);
    sOffset = (sLength == 33 ? 1 : 0);
    if (32 - rLength > 0) os_memset(signature + 1, 0, 32 - rLength);
    os_memmove(signature + 1 + (32 > rLength ? 32 - rLength : 0), tempSignature + 4 + rOffset, (32 > rLength ? rLength : 32));
    if (32 - sLength > 0) os_memset(signature + 1 + 32, 0, 32 - sLength);
    os_memmove(signature + 1 + 32 + (32 > sLength ? 32 - sLength : 0), tempSignature + 4 + rLength + 2 + sOffset, (32 > sLength ? sLength : 32));
    *signature_length = 65;


#ifdef TESTING_ENABLED
    return cx_ecdsa_verify(
            &publicKey,
            CX_LAST,
            CX_SHA256,
            message_digest,
            CX_SHA256_SIZE,
            tempSignature,
            signatureLength);
#else
    return 1;
#endif
}

int sign_secp256k1_eos(const uint8_t *message,
                   unsigned int message_length,
                   uint8_t *signature,
                   unsigned int signature_capacity,
                   unsigned int *signature_length,
                   cx_ecfp_private_key_t *privateKey) {
    uint8_t message_digest[CX_SHA256_SIZE];
    unsigned int i;
    for (i = 0; i < CX_SHA256_SIZE; i++) {
        if (i < message_length) {
            message_digest[i] = message[i];
        }
        else {
            message_digest[i] = 0;
        }
    }

    uint8_t tempSignature[100];
    uint8_t signatureLength;
    uint8_t V[33];
    uint8_t K[32];
    int tries = 0;

    // Loop until a candidate matching the canonical signature is found
    for (;;) {
        if (tries == 0) {
            rng_rfc6979(tempSignature, message_digest, privateKey->d, privateKey->d_len, SECP256K1_N, 32, V, K);
        }
        else {
            rng_rfc6979(tempSignature, message_digest, NULL, 0, SECP256K1_N, 32, V, K);
        }
        uint32_t info;
        signatureLength = cx_ecdsa_sign(privateKey, CX_NO_CANONICAL | CX_RND_PROVIDED | CX_LAST, CX_SHA256,
                           message_digest, 32, 
                           tempSignature, sizeof(tempSignature),
                           &info);
        if ((info & CX_ECCINFO_PARITY_ODD) != 0) {
            tempSignature[0] |= 0x01;
        }
        signature[0] = 27 + 4 + (tempSignature[0] & 0x01);
        ecdsa_der_to_sig(tempSignature, signature + 1);
        if (check_canonical(signature + 1)) {
            *signature_length = 65;
            break;
        }
        else
        {
            tries++;
        }
    }

    return 1;
}

