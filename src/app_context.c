#include "app_context.h"
#include "crypto.h"
#include "common/utils.h"
#include "tx/tx.h"
#include <string.h>

int app_context_init(app_context_t* context, uint8_t* buffer, signing_type_t type) {
    if (!context || !buffer) {
        return 1;
    }

    context->buffer = buffer;
    context->signing_type = type;
    return app_context_update(context);
}

int app_context_update(app_context_t* context) {
    int32_t offset = 0;

    // coin symbol
    uint8_t len = context->buffer[offset++];
    if (len > MAX_COIN_TYPE_LENGTH) {
        return 1;
    }
    char coin_symbol[MAX_COIN_TYPE_LENGTH + 1];
    memset(coin_symbol, 0, sizeof(coin_symbol));
    os_memmove(coin_symbol, context->buffer + offset, len);
    context->coin = get_coin(coin_symbol);
    context->curr = get_curr_info(coin_symbol);
    offset += len;

    if (!context->coin || !context->curr) { // not supported coin
        return 1;
    }

    // tx raw
    offset += read_uint32_le(context->buffer + offset, &context->tx_raw_len);
    context->tx_raw = context->buffer + offset;
    offset += context->tx_raw_len;

    // tx
    memset(&context->tx, 0, sizeof(context->tx));
    return tx_init(context);
}

int app_context_reset(app_context_t* context) {
    memset(context, 0, sizeof(app_context_t));
    return 0;
}

int app_context_set_user_data(app_context_t* context, uint8_t* user_data,
                              uint32_t user_data_len) {
    context->user_data = user_data;
    context->user_data_len = user_data_len;
    return 0;
}

int app_context_sign_tx(app_context_t* context, uint32_t* path, uint8_t depth, uint32_t index, uint8_t* signature, int signature_capacity, int* signature_length) {
    return tx_sign(context, path, depth, index, signature, signature_capacity, signature_length);
}
