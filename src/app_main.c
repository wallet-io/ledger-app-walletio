/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2019 WalletIO
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

#include "app_main.h"
#include "view.h"
#include "lib/message.h"
#include "signature.h"
#include "zxmacros.h"

#include <os_io_seproxyhal.h>
#include <os.h>

#include <string.h>

#ifdef TESTING_ENABLED
// Generate using always the same private data
// to allow for reproducible results
const uint8_t privateKeyDataTest[] = {
        0x75, 0x56, 0x0e, 0x4d, 0xde, 0xa0, 0x63, 0x05,
        0xc3, 0x6e, 0x2e, 0xb5, 0xf7, 0x2a, 0xca, 0x71,
        0x2d, 0x13, 0x4c, 0xc2, 0xa0, 0x59, 0xbf, 0xe8,
        0x7e, 0x9b, 0x5d, 0x55, 0xbf, 0x81, 0x3b, 0xd4
};
#endif

uint8_t bip32_depth;
uint32_t bip32_path[5];

uint32_t unlock_key;
uint32_t sign_counter;

sigtype_t current_sigtype;

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

unsigned char io_event(unsigned char channel) {
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_FINGER_EVENT: //
            UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
            if (!UX_DISPLAYED())
                UX_DISPLAYED_EVENT();
            break;

        case SEPROXYHAL_TAG_TICKER_EVENT: { //
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
                    if (UX_ALLOWED) {
                        UX_REDISPLAY();
                    }
            });
            break;
        }

            // unknown events are acknowledged
        default:
            UX_DEFAULT_EVENT();
            break;
    }
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }
    return 1; // DO NOT reset the current APDU transport
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;

            // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0; // nothing received from the master so far (it's a tx
                // transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                              sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(INVALID_PARAMETER);
    }
    return 0;
}

void app_init() {
    io_seproxyhal_init();
    USB_power(0);
    USB_power(1);
    view_idle(0);

    unlock_key = 0;
    sign_counter = 0;
}

// extract_bip32 extracts the bip32 path from the apdu buffer
bool extract_bip32(uint8_t *depth, uint32_t path[5], uint32_t rx, uint32_t offset) {
    if (rx < offset + 1) {
        return 0;
    }

    *depth = G_io_apdu_buffer[offset];
    const uint16_t req_offset = 4 * *depth + 1 + offset;

    if (rx < req_offset || *depth > 5 || *depth < 1) {
        return 0;
    }
    memcpy(path, G_io_apdu_buffer + offset + 1, *depth * 4);
    return 1;
}

// validate_bip32 checks the given bip32 path against an expected one
bool validate_bip32(uint8_t depth, uint32_t path[5]) {  // path is 10 bytes for compatibility
    if (depth < 1 || depth > 5) {
        return 0;
    }

    return 1;
}

void get_pubkey(cx_ecfp_public_key_t *publicKey, uint8_t * chainCode, sigtype_t curve) {
    cx_ecfp_private_key_t privateKey;
    uint8_t privateKeyData[32];

    // Generate keys
    cx_curve_t cxCurve;
    if (curve == SECP256K1) {
        cxCurve = CX_CURVE_256K1;
    }
    else if (curve == ED25519) {
        cxCurve = CX_CURVE_Ed25519;
    }
    else {
        THROW(APDU_CODE_DATA_INVALID);
    }
    
    os_perso_derive_node_bip32(
            cxCurve,
            bip32_path, bip32_depth,
            privateKeyData, chainCode);
    keys(publicKey, &privateKey, privateKeyData, cxCurve);
    memset(privateKeyData, 0, sizeof(privateKeyData));
    memset(&privateKey, 0, sizeof(privateKey));
}
//region View Transaction Handlers

void buffer2hex(const uint8_t *src, int src_len, char *dest, int dest_len) {
    uint8_t map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    int i;
    for(i = 0; i < src_len && i < dest_len / 2; i++) {
        uint8_t b = (src[i] >> 4) & 0x0f;
        uint8_t l = src[i] & 0x0f;
        dest[i * 2] = map[b];
        dest[i * 2 + 1] = map[l];
    }
    dest[i * 2] = 0;

    return;
}

int sign_getData(
        char *title, int max_title_length,
        char *key, int max_key_length,
        char *value, int max_value_length,
        int page_index,
        int chunk_index,
        int *page_count_out,
        int *chunk_count_out) {

    *page_count_out = 1;

    switch (current_sigtype) {
        case SECP256K1_EOS:
        case SECP256K1:
            snprintf(title, max_title_length, "SIGN");
            break;
        default:
            snprintf(title, max_title_length, "INVALID!");
            break;
    }

    strcpy(key, "Message");
    buffer2hex(message_get_buffer(), message_get_buffer_length(), value, max_value_length);
    
    *chunk_count_out = 1;

    return 0;
}

void sign_accept() {
    // Generate keys
    cx_ecfp_public_key_t publicKey;
    cx_ecfp_private_key_t privateKey;
    uint8_t privateKeyData[32];

    unsigned int length = 0;
    int result = 0;
    switch (current_sigtype) {
        case SECP256K1_EOS:
        case SECP256K1:
            os_perso_derive_node_bip32(
                    CX_CURVE_256K1,
                    bip32_path, bip32_depth,
                    privateKeyData, NULL);

            keys(&publicKey, &privateKey, privateKeyData, CX_CURVE_256K1);
            memset(privateKeyData, 0, 32);

            if (current_sigtype == SECP256K1_EOS) {
                result = sign_secp256k1_eos(
                    message_get_buffer(),
                    message_get_buffer_length(),
                    G_io_apdu_buffer,
                    IO_APDU_BUFFER_SIZE,
                    &length,
                    &privateKey);
            }
            else {
                result = sign_secp256k1(
                    message_get_buffer(),
                    message_get_buffer_length(),
                    G_io_apdu_buffer,
                    IO_APDU_BUFFER_SIZE,
                    &length,
                    &privateKey);
            }
            
            break;
        default:
            THROW(APDU_CODE_INS_NOT_SUPPORTED);
            break;
    }
    if (result == 1) {
        set_code(G_io_apdu_buffer, length, APDU_CODE_OK);
        io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, length + 2);
        view_idle(0);
    } else {
        set_code(G_io_apdu_buffer, length, APDU_CODE_SIGN_VERIFY_ERROR);
        io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, length + 2);
        view_idle(0);
    }
}

void sign_reject() {
    set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    view_idle(0);
}

int unlock_getData(
        char *title, int max_title_length,
        char *key, int max_key_length,
        char *value, int max_value_length,
        int page_index,
        int chunk_index,
        int *page_count_out,
        int *chunk_count_out) {

    *page_count_out = 1;

    snprintf(title, max_title_length, "UNLOCK");
    snprintf(key, max_title_length, "Key: %d", unlock_key);
    snprintf(value, max_title_length, "Count: %d", sign_counter);

    *chunk_count_out = 1;

    return 0;
}

void unlock_accept() {
    set_code(G_io_apdu_buffer, 0, APDU_CODE_OK);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    view_idle(0);
}

void unlock_reject() {
    unlock_key = 0;
    sign_counter = 0;
    set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    view_idle(0);
}

//endregion

void handleApdu(volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    uint16_t sw = 0;

    BEGIN_TRY
    {
        TRY
        {
            if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                THROW(APDU_CODE_CLA_NOT_SUPPORTED);
            }

            if (rx < 5) {
                THROW(APDU_CODE_WRONG_LENGTH);
            }
            uint8_t ins = G_io_apdu_buffer[OFFSET_INS];
            switch (ins) {
                case INS_GET_VERSION: {
                    unsigned int UX_ALLOWED = (ux.params.len != BOLOS_UX_IGNORE && ux.params.len != BOLOS_UX_CONTINUE);

#ifdef TESTING_ENABLED
                    G_io_apdu_buffer[0] = 0xFF;
#else
                    G_io_apdu_buffer[0] = 0;
#endif
                    G_io_apdu_buffer[1] = LEDGER_MAJOR_VERSION;
                    G_io_apdu_buffer[2] = LEDGER_MINOR_VERSION;
                    G_io_apdu_buffer[3] = LEDGER_PATCH_VERSION;
                    G_io_apdu_buffer[4] = !UX_ALLOWED;

                    *tx += 5;
                    THROW(APDU_CODE_OK);
                    break;
                }

                case INS_UNLOCK: {
                    if (rx < OFFSET_DATA + 8) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }
                    memcpy(&unlock_key, G_io_apdu_buffer + OFFSET_DATA, 4);
                    memcpy(&sign_counter, G_io_apdu_buffer + OFFSET_DATA + 4, 4);

                    view_set_handlers(unlock_getData, unlock_accept, unlock_reject);
                    view_unlock_confirm(0);

                    *flags |= IO_ASYNCH_REPLY;
                    *tx = 0;

                    break;
                }

                case INS_PUBLIC_KEY_ED25519:
                case INS_PUBLIC_KEY_SECP256K1: {
                    if (!extract_bip32(&bip32_depth, bip32_path, rx, OFFSET_DATA)) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }
                    if (!validate_bip32(bip32_depth, bip32_path)) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    cx_ecfp_public_key_t publicKey;
                    uint8_t chainCode[32];
                    sigtype_t curve = SECP256K1;
                    if (ins == INS_PUBLIC_KEY_ED25519) {
                        curve = ED25519;
                    }

                    get_pubkey(&publicKey, chainCode, curve);

                    os_memmove(G_io_apdu_buffer, publicKey.W, 65);
                    *tx += 65;

                    os_memmove(G_io_apdu_buffer + 65, chainCode, 32);
                    *tx += 32;

                    THROW(APDU_CODE_OK);
                    break;
                }

                case INS_SIGN_SECP256K1:
                case INS_SIGN_SECP256K1_2:
                case INS_SIGN_SECP256K1_EOS:
                case INS_SIGN_SECP256K1_EOS_2: {
                    if (ins == INS_SIGN_SECP256K1_EOS || ins == INS_SIGN_SECP256K1_EOS_2) {
                        current_sigtype = SECP256K1_EOS;
                    }
                    else {
                        current_sigtype = SECP256K1;
                    }
                    
                    // data: 4 bytes unlock_key + 1 byte path-depth + 4 * path-depth bytes + 32 bytes message hash
                    if (rx != OFFSET_DATA + 4 + 1 + G_io_apdu_buffer[OFFSET_DATA + 4] * 4 + 32) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    // extract unlock key and then check it
                    uint32_t now_unlock_key;
                    memcpy(&now_unlock_key, G_io_apdu_buffer + OFFSET_DATA, 4);

                    // extract bip32 path and then check it
                    if (!extract_bip32(&bip32_depth, bip32_path, rx, OFFSET_DATA + 4)) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }
                    if (!validate_bip32(bip32_depth, bip32_path)) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    // extract message hash
                    message_initialize();
                    message_reset();
                    uint32_t offset = OFFSET_DATA + 4 + 1 + bip32_depth * 4;
                    if (message_append(&(G_io_apdu_buffer[offset]), rx - offset) != rx - offset) {
                        THROW(APDU_CODE_OUTPUT_BUFFER_TOO_SMALL);
                    }

                    if (ins == INS_SIGN_SECP256K1_2 || ins == INS_SIGN_SECP256K1_EOS_2) {
                        if (unlock_key == 0 || unlock_key != now_unlock_key || sign_counter <= 0) {
                            THROW(APDU_CODE_CONDITIONS_NOT_SATISFIED);
                        }

                        sign_accept();
                        sign_counter--;
                    }
                    else {
                        view_set_handlers(sign_getData, sign_accept, sign_reject);
                        view_sign_confirm(0);

                        *flags |= IO_ASYNCH_REPLY;
                        *tx = 0;
                    }

                    break;
                }

#ifdef TESTING_ENABLED
                case INS_PUBLIC_KEY_SECP256K1_TEST: {
                    // Generate key
                    cx_ecfp_public_key_t publicKey;
                    cx_ecfp_private_key_t privateKey;
                    keys(&publicKey, &privateKey, privateKeyDataTest, CX_CURVE_256K1);
                    os_memmove(G_io_apdu_buffer, publicKey.W, 65);
                    *tx += 65;

                    THROW(APDU_CODE_OK);
                    break;
                }
#endif

                default:
                    THROW(APDU_CODE_INS_NOT_SUPPORTED);
            }
        }
        CATCH(EXCEPTION_IO_RESET)
        {
            THROW(EXCEPTION_IO_RESET);
        }
        CATCH_OTHER(e)
        {
            switch (e & 0xF000) {
                case 0x6000:
                case APDU_CODE_OK:
                    sw = e;
                    break;
                default:
                    sw = 0x6800 | (e & 0x7FF);
                    break;
            }
            G_io_apdu_buffer[*tx] = sw >> 8;
            G_io_apdu_buffer[*tx + 1] = sw;
            *tx += 2;
        }
        FINALLY
        {
        }
    }
    END_TRY;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void app_main() {
    volatile uint32_t rx = 0, tx = 0, flags = 0;

    for (;;) {
        volatile uint16_t sw = 0;

        BEGIN_TRY;
        {
            TRY;
            {
                rx = tx;
                tx = 0;
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                if (rx == 0)
                    THROW(APDU_CODE_EMPTY_BUFFER);

                handleApdu(&flags, &tx, rx);
            }
            CATCH_OTHER(e);
            {
                switch (e & 0xF000) {
                    case 0x6000:
                    case 0x9000:
                        sw = e;
                        break;
                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY;
            {}
        }
        END_TRY;
    }
}

#pragma clang diagnostic pop
