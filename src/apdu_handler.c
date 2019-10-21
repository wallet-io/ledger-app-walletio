#include "apdu_handler.h"

#include <os.h>
#include <os_io_seproxyhal.h>
#include <string.h>
#include "view/view.h"

#include "btc/btc_transaction.h"
#include "common/utils.h"
#include "crypto.h"
#include "tx_buffer.h"

void handle_apdu(app_context_t *context, volatile uint32_t *flags,
                 volatile uint32_t *tx, uint32_t rx) {
    uint16_t sw = 0;

    BEGIN_TRY {
        TRY {
            PRINTF("\n---------------------\n");
            PRINTF(">>>>>>>>>>>>>>>>>>>>>\n");
            PRINTF("apdu in: %.*H\n", rx, G_io_apdu_buffer);
            PRINTF("\n<<<<<<<<<<<<<<<<<<<<<\n");
            if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                THROW(APDU_CODE_CLA_NOT_SUPPORTED);
            }

            if (rx < 5) {
                THROW(APDU_CODE_WRONG_LENGTH);
            }
            uint8_t ins = G_io_apdu_buffer[OFFSET_INS];
            switch (ins) {
                case INS_GET_VERSION: {
                    unsigned int UX_ALLOWED =
                        (ux.params.len != BOLOS_UX_IGNORE &&
                         ux.params.len != BOLOS_UX_CONTINUE);
                    G_io_apdu_buffer[0] = 0;
                    G_io_apdu_buffer[1] = LEDGER_MAJOR_VERSION;
                    G_io_apdu_buffer[2] = LEDGER_MINOR_VERSION;
                    G_io_apdu_buffer[3] = LEDGER_PATCH_VERSION;
                    G_io_apdu_buffer[4] = !UX_ALLOWED;

                    *tx += 5;
                    THROW(APDU_CODE_OK);
                    break;
                }

                case INS_PUBLIC_KEY: {
                    if (rx < OFFSET_DATA + 2) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    uint32_t offset = OFFSET_DATA;
                    sig_type_t sig_type = G_io_apdu_buffer[offset++];
                    if (sig_type != SECP256K1 && sig_type != SECP256R1 &&
                        sig_type != ED25519) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    uint8_t depth = G_io_apdu_buffer[offset++];
                    if (depth < 1 || depth > MAX_BIP32_DEPTH) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    if (rx < offset + 4 * depth) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    uint32_t path[MAX_BIP32_DEPTH];
                    for (int i = 0; i < depth; i++) {
                        offset +=
                            read_uint32_le(G_io_apdu_buffer + offset, path + i);
                    }

                    uint8_t public_key_data[65];
                    uint8_t chain_code[32];
                    get_pub_key(public_key_data, chain_code, sig_type, path,
                                depth);

                    os_memmove(G_io_apdu_buffer, public_key_data, 65);
                    *tx += 65;

                    os_memmove(G_io_apdu_buffer + 65, chain_code, 32);
                    *tx += 32;

                    THROW(APDU_CODE_OK);
                    break;
                }

                case INS_SIGN_TX:
                case INS_SIGN_MESSAGE: {
                    int pkg_index = G_io_apdu_buffer[OFFSET_PCK_INDEX];
                    int pkg_count = G_io_apdu_buffer[OFFSET_PCK_COUNT];

                    if (pkg_index == 0) {
                        tx_buffer_initialize();
                        tx_buffer_reset();
                    }

                    tx_buffer_append(G_io_apdu_buffer + OFFSET_DATA,
                                     rx - OFFSET_DATA);

                    if (pkg_index == pkg_count - 1) {
                        int ret = app_context_init(context, tx_buffer_get_buffer(), ins == INS_SIGN_TX ? SIGN_TX : (ins == INS_SIGN_MESSAGE ? SIGN_MESSAGE : 0));
                        if (ret != 0) {
                            THROW(APDU_CODE_DATA_INVALID);
                        }
                    } else {
                        THROW(APDU_CODE_OK);
                        break;
                    }

                    // check context
                    if (!context->coin) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    view_tx_show(context);

                    *flags |= IO_ASYNCH_REPLY;
                    *tx = 0;
                    break;
                }

                case INS_REQUEST_SIG: {
                    if (!context->accept_signing) {
                        PRINTF("INS_REQUEST_SIG has been rejected");
                        THROW(APDU_CODE_UNKNOWN);
                    }

                    uint32_t offset = OFFSET_DATA;

                    uint8_t depth = G_io_apdu_buffer[offset++];
                    if (depth < 1 || depth > MAX_BIP32_DEPTH) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    int path_len = 4 * depth;
                    if (rx < offset + path_len) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    uint32_t path[MAX_BIP32_DEPTH];
                    for (int i = 0; i < depth; i++) {
                        offset +=
                            read_uint32_le(G_io_apdu_buffer + offset, path + i);
                    }

                    int index;
                    offset += read_uint32_le(G_io_apdu_buffer + offset, &index);

                    uint8_t script_len = G_io_apdu_buffer[offset++];
                    if (rx < offset + script_len) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    app_context_set_user_data(
                        context, G_io_apdu_buffer + offset, script_len);

                    int signature_length = 0;

                    if (app_context_sign_tx(context, path, depth, index, G_io_apdu_buffer, 100, &signature_length) != 0) {
                        THROW(APDU_CODE_UNKNOWN);
                    }
                    *tx += signature_length;

                    THROW(APDU_CODE_OK);
                    break;
                }

                default:
                    THROW(APDU_CODE_INS_NOT_SUPPORTED);
            }
        }
        CATCH(EXCEPTION_IO_RESET) { THROW(EXCEPTION_IO_RESET); }
        CATCH_OTHER(e) {
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
        FINALLY {}
    }
    END_TRY;
}
