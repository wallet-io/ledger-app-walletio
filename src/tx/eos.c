#include "btc.h"
#include "crypto.h"
#include "../app_context.h"

#include "tx.h"
#include "eos/eos_transaction.h"
#include "common/utils.h"
#include "common/tx_helper.h"

#include "os.h"

int eos_init_tx(app_context_t* context) {
    return eos_tx_init(&context->tx.tx_eos, context->tx_raw, context->tx_raw_len);
}

int eos_get_tx_info_page_size(app_context_t* context) {
    return context->tx.tx_eos.action_len;
}

int eos_get_tx_info_page(app_context_t* context, uint32_t index, tx_info_page_t* page) {
    int ret;

    eos_tx_action_t action;
    ret = eos_tx_get_action(&context->tx.tx_eos, 0, &action);

    int len;
    char name[16];

    memset(name, 0, sizeof(name));
    ret = eos_decode_name(action.name, name, &len);
    name[len] = 0;

    if (strcmp(name, "transfer") == 0) {
        eos_tx_action_transfer_t transfer;
        ret = eos_tx_get_action_transfer(&action, &transfer);

        memset(page->to, 0, 32);
        ret = eos_decode_name(transfer.to, page->to, &len);
        uint8_t cut_out[10]={0};
        str_cut(transfer.memo, transfer.memo_len, cut_out, sizeof(cut_out) - 1, 0x01 | 0x02);
        snprintf(page->to + len, sizeof(page->to) - len, " / %s", cut_out);
        snprintf(page->amount_str, sizeof(page->amount_str), "%d", (uint32_t)transfer.amount);
    }
    else {
        strcpy(page->to, "UNKOWN");
        strcpy(page->amount_str, "0");
    }

    return ret;
}

int eos_hash_256(void* param, uint8_t* data, size_t len, int step) {
    PRINTF("eos_hash_256: %.*h\n", len, data);
    app_context_t * context = (app_context_t*)param;
    if (step == STEP_BEGIN) {
        cx_sha256_init(&context->hash_context.hash.sha256);
    }

    if (len > 0) {
        cx_hash(&context->hash_context.hash.sha256.header, 0, data, len, NULL, 0);
    }

    if (step == STEP_END) {
        cx_hash(&context->hash_context.hash.sha256.header, CX_LAST, NULL, 0, context->hash_context.data.m32, 32);
    }
    
    return 0;
}

int eos_sign_tx(app_context_t* context, uint32_t* path, uint8_t depth, uint32_t index, uint8_t* signature,int signature_capacity, int* signature_length) {
    int ret = 1;

    ret = eth_tx_hash_for_signature(&context->tx.tx_eth, &eos_hash_256, (void*)context);
    ret = sign(context->hash_context.data.m32, 32, signature, signature_capacity, signature_length, SECP256K1, path, depth, 1);
    return ret;
}

int eos_magic_hash(app_context_t* context) {
    const char* message_prefix = (const char*)PIC(context->curr->message_prefix);
    uint8_t prefix_len = (uint8_t)strlen(message_prefix);
    btc_hash_256((void*)context, &prefix_len, 1, STEP_BEGIN);
    btc_hash_256((void*)context, message_prefix, prefix_len, STEP_CONTINUOUS);

    uint8_t buffer[10];
    int message_len = encode_var_int(buffer, context->tx_raw_len);
    btc_hash_256((void*)context, buffer, message_len, STEP_CONTINUOUS);
    btc_hash_256((void*)context, context->tx_raw, context->tx_raw_len, STEP_END);

    return 0;
}

int eos_sign_message(app_context_t* context, uint32_t* path, uint8_t depth, uint8_t* signature,int signature_capacity, int* signature_length) {
    eos_magic_hash(context);
    return sign(context->hash_context.data.m32, 32, signature, signature_capacity, signature_length, SECP256K1, path, depth, 0);
}
