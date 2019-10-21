#include "tx.h"
#include "btc.h"
#include "eth.h"
#include "eth_m.h"
#include "eos.h"

#include "../app_context.h"

int tx_init(app_context_t* context) {
    if (context->signing_type == SIGN_MESSAGE) {
        return 0;
    }

    int ret = 1;
    if (strcmp(context->coin->cate, "BTC") == 0) {
        ret = btc_init_tx(context);
    }
    else if (strcmp(context->coin->cate, "ETH") == 0) {
        ret = eth_init_tx(context);
    }
    else if (strcmp(context->coin->cate, "ETHM") == 0) {
        ret = eth_m_init_tx(context);
    }
    else if (strcmp(context->coin->cate, "EOS") == 0) {
        ret = eos_init_tx(context);
    }
    return ret;
}

int tx_sign(app_context_t* context, uint32_t* path, uint8_t depth, uint32_t index, uint8_t* signature, int signature_capacity, int* signature_length) {
    int ret = 1;
    PRINTF("tx_sign: %d\n", context->signing_type);
    PRINTF("tx raw:%.*H\n", context->tx_raw_len, context->tx_raw);
    PRINTF("symbol:%s\n", context->coin->symbol);
    PRINTF("cate:%s\n", context->coin->cate);

    if (strcmp(context->coin->cate, "BTC") == 0) {
        if (context->signing_type == SIGN_TX) {
            ret = btc_sign_tx(context, path, depth, index, signature, signature_capacity, signature_length);
        }
        else if (context->signing_type == SIGN_MESSAGE) {
            ret = btc_sign_message(context, path, depth, signature, signature_capacity, signature_length);
        }
    }
    else if (strcmp(context->coin->cate, "ETH") == 0 || strcmp(context->coin->cate, "ETHM") == 0) {
        if (context->signing_type == SIGN_TX) {
            ret = eth_sign_tx(context, path, depth, index, signature, signature_capacity, signature_length);
        }
        else if (context->signing_type == SIGN_MESSAGE) {
            ret = eth_sign_message(context, path, depth, signature, signature_capacity, signature_length);
        }
    }
    else if (strcmp(context->coin->cate, "EOS") == 0) {
        if (context->signing_type == SIGN_TX) {
            ret = eos_sign_tx(context, path, depth, index, signature, signature_capacity, signature_length);
        }
        else if (context->signing_type == SIGN_MESSAGE) {
            ret = eos_sign_message(context, path, depth, signature, signature_capacity, signature_length);
        }
    }
    return ret;
}

int get_tx_info_page_size(app_context_t* context) {
    if (context->signing_type == SIGN_MESSAGE) {
        return 1;
    }

    int ret = 1;
    if (strcmp(context->coin->cate, "BTC") == 0) {
        ret = btc_get_tx_info_page_size(context);
    }
    else if (strcmp(context->coin->cate, "ETH") == 0) {
        ret = eth_get_tx_info_page_size(context);
    }
    else if (strcmp(context->coin->cate, "ETHM") == 0) {
        ret = eth_m_get_tx_info_page_size(context);
    }
    else if (strcmp(context->coin->cate, "EOS") == 0) {
        ret = eos_get_tx_info_page_size(context);
    }
    return ret * 2;
}

int get_tx_info_page(app_context_t* context, uint32_t index, tx_info_page_t* page) {
    if (context->signing_type == SIGN_MESSAGE) {
        strcpy(page->amount_str, context->coin->symbol);
        snprintf(page->to, sizeof(page->to), "%.*s", context->tx_raw_len, context->tx_raw);
        return 0;
    }

    int ret = 1;
    if (strcmp(context->coin->cate, "BTC") == 0) {
        ret = btc_get_tx_info_page(context, index, page);
    }
    else if (strcmp(context->coin->cate, "ETH") == 0) {
        ret = eth_get_tx_info_page(context, index, page);
    }
    else if (strcmp(context->coin->cate, "ETHM") == 0) {
        ret = eth_m_get_tx_info_page(context, index, page);
    }
    else if (strcmp(context->coin->cate, "EOS") == 0) {
        ret = eos_get_tx_info_page(context, index, page);
    }
    return ret;
}