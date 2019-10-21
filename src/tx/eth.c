#include "btc.h"
#include "crypto.h"
#include "../app_context.h"

#include "tx.h"
#include "eth/eth_transaction.h"
#include "common/utils.h"
#include "common/tx_helper.h"

#include "os.h"

static uint8_t global_tmp[68];

int eth_init_tx(app_context_t* context) {
    return eth_tx_init(&context->tx.tx_eth, context->tx_raw, context->tx_raw_len);
}

int eth_get_tx_info_page_size(app_context_t* context) {
    uint8_t data_len = 0;
    eth_tx_get_data(&context->tx.tx_eth, global_tmp, &data_len, sizeof(global_tmp));
    if (data_len != 0)
        return 2;
    else
        return 1;
}

int eth_get_tx_info_page(app_context_t* context, uint32_t index, tx_info_page_t* page) {
    int ret;
    uint8_t out_len = 0;
    uint32_t address_len;

    if (index == 0){
        ret = eth_tx_get_to(&context->tx.tx_eth, global_tmp, &address_len, sizeof(global_tmp));
        memset(page->to, 0, sizeof(page->to));
        page->to[0] = '0';
        page->to[1] = 'x';
        address_len = buffer_to_hex(global_tmp, address_len, page->to + 2, sizeof(page->to));
        page->to[2 + address_len * 2] = '\0';

        uint32_t value_len;
        memset(global_tmp, 0, sizeof(global_tmp));
        ret = eth_tx_get_value(&context->tx.tx_eth, global_tmp, &value_len, sizeof(global_tmp));
        if (value_len == 0) {
            strcpy(page->amount_str, "0");
        } else {
            uint8_t buffer[value_len * 2];
            out_len = uint8_extension(global_tmp, (uint8_t)value_len, buffer, sizeof(buffer));
            out_len = base_convert(buffer, out_len, page->amount_str, sizeof(page->amount_str), 16, 10);
            num2str(page->amount_str, out_len);
        }
    } else if(index == 1){
        uint32_t data_len;
        ret = eth_tx_get_data(&context->tx.tx_eth, global_tmp, &data_len, sizeof(global_tmp));

        uint8_t code_flag[] = {0xa9, 0x05, 0x9c, 0xbb};
        if (memcmp(global_tmp, code_flag, 4) != 0){
            data_len = buffer_to_hex(global_tmp, data_len, page->to, sizeof(page->to));
            if (data_len > sizeof(page->to)) data_len = sizeof(page->to);
            page->to[data_len] = 0;
            memcpy(page->amount_str,page->to,data_len);
            return ret;
        }
        
        page->to[0] = '0';
        page->to[1] = 'x';
        buffer_to_hex(global_tmp + 4 + 12, 20, page->to + 2, sizeof(page->to));
        page->to[2 + 20 * 2] = '\0';

        uint8_t avaiable_amount_index = 4 + 32;
        while (global_tmp[avaiable_amount_index] ==0){
            avaiable_amount_index++;
        }
        uint8_t avaiable_len = sizeof(global_tmp) - avaiable_amount_index;
        uint8_t temp1[avaiable_len * 2];

        out_len = uint8_extension(global_tmp + avaiable_amount_index,avaiable_len , temp1, sizeof(temp1));
        out_len = base_convert(temp1, out_len, page->amount_str, sizeof(page->amount_str), 16, 10);
        num2str(page->amount_str, out_len);
    }
    return ret;
}

int eth_hash_256(void* param, uint8_t* data, size_t len, int step) {
    PRINTF("eth_hash_256: %.*h\n", len, data);
    app_context_t * context = (app_context_t*)param;
    if (step == STEP_BEGIN) {
        cx_keccak_init(&context->hash_context.hash.sha3, 256);
    }

    if (len > 0) {
        cx_hash((cx_hash_t *)&context->hash_context.hash.sha3, 0, data, len, NULL, 0);
    }

    if (step == STEP_END) {
        cx_hash((cx_hash_t *)&context->hash_context.hash.sha3, CX_LAST, NULL, 0, context->hash_context.data.m32, 32);
    }
    
    return 0;
}

int eth_sign_tx(app_context_t* context, uint32_t* path, uint8_t depth, uint32_t index, uint8_t* signature,int signature_capacity, int* signature_length) {
    int ret = 1;

    ret = eth_tx_hash_for_signature(&context->tx.tx_eth, &eth_hash_256, (void*)context);
    ret = sign(context->hash_context.data.m32, 32, signature, signature_capacity, signature_length, SECP256K1, path, depth, 0);
    return ret;
}

int eth_magic_hash(app_context_t* context) {
    const char* message_prefix = (const char*)PIC(context->curr->message_prefix);
    uint8_t prefix_len = (uint8_t)strlen(message_prefix);
    eth_hash_256((void*)context, &prefix_len, 1, STEP_BEGIN);
    eth_hash_256((void*)context, message_prefix, prefix_len, STEP_CONTINUOUS);

    uint8_t buffer[10] = {0};
    snprintf(buffer, sizeof(buffer), "%d", context->tx_raw_len);
    int message_len = strlen(buffer);
    eth_hash_256((void*)context, buffer, message_len, STEP_CONTINUOUS);
    eth_hash_256((void*)context, context->tx_raw, context->tx_raw_len, STEP_END);

    return 0;
}

int eth_sign_message(app_context_t* context, uint32_t* path, uint8_t depth, uint8_t* signature,int signature_capacity, int* signature_length) {
    eth_magic_hash(context);
    PRINTF("eth_sign_message hash: %.*h\n", 32, context->hash_context.data.m32);
    return sign(context->hash_context.data.m32, 32, signature, signature_capacity, signature_length, SECP256K1, path, depth, 0);
}
