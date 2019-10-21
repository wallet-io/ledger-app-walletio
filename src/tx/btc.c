#include "btc.h"
#include "../app_context.h"
#include "crypto.h"

#include "btc/btc_transaction.h"
#include "common/tx_helper.h"
#include "tx.h"

#include "../address.h"
#include "../lib/common_utils.h"
#include "os.h"

typedef enum {
    PREFIX = 0,
    TX_TYPE,
    PROPERTY_ID,
    AMOUNT
} tether_field_t;

typedef enum {
    VM_VERSION = 0,
    GAS_LIMIT,
    GAS_PRICE,
    DATAHEX,
    CONTRACT_ADDRESS,
    OP_CALL
} qtum_field_t;

typedef enum {
    QTUM_TRANSFER_HEX = 0,
    QTUM_TO_ADDRESS,
    QTUM_VALUE,
} qtum_transfer_token_data_t;

static uint8_t btc_decode_tether(const uint8_t* src, uint32_t src_len, const tether_field_t index, uint8_t* out, uint8_t out_len);
static uint8_t btc_decode_qtum(const uint8_t* buffer, uint32_t buffer_len, const qtum_field_t field_index, uint8_t** out);
static uint8_t btc_qtum_decode_token_data(const uint8_t* buffer, uint32_t buffer_len, const qtum_transfer_token_data_t field_index, uint8_t** out);
static bool handle_btc_output(app_context_t* context, tx_info_page_t* page, btc_tx_output_t* tx_out, int* ret);
static bool handle_tether_output(app_context_t* context, tx_info_page_t* page, btc_tx_output_t* tx_out, int* ret);
static bool handle_qtum_output(app_context_t* context, tx_info_page_t* page, btc_tx_output_t* tx_out, int* ret);

int btc_init_tx(app_context_t* context) {
    context->tx.tx_btc.mask = 0;
    if (!strcmp(context->coin->symbol, "XVG")) {
        context->tx.tx_btc.mask |= BTC_TX_MASK_NTIME;
    }
    return btc_tx_init(&context->tx.tx_btc, context->tx_raw, context->tx_raw_len);
}

int btc_get_tx_info_page_size(app_context_t* context) {
    return context->tx.tx_btc.output_len;
}

int btc_get_tx_info_page(app_context_t* context, uint32_t index, tx_info_page_t* page) {
    int ret = 0;
    btc_tx_output_t tx_out;

    btc_tx_get_output(&context->tx.tx_btc, index, &tx_out);

    if (!strcmp(context->coin->symbol, "QTUM")) {
        PRINTF("index:%d,script_len:%d\n", index, (uint32_t)tx_out.script_len);
        bool res = handle_qtum_output(context, page, &tx_out, &ret);
        if (res == true)
            return ret;
    }

    uint8_t tmp[10];
    memset(tmp, 0, sizeof(tmp));
    uint8_t len = btc_decode_tether(tx_out.script + 2, (uint32_t)tx_out.script_len, PREFIX, tmp, sizeof(tmp));
    PRINTF("len: %d,%d\n", len, (uint32_t)tx_out.script_len);
    uint8_t tether_prefix[] = {0x6f, 0x6d, 0x6e, 0x69};

    // if it's a ustd tx output
    if (len != 0 && memcmp(tmp, tether_prefix, sizeof(tether_prefix) - 1) == 0) {
        handle_tether_output(context, page, &tx_out, &ret);
        // if it's a general btc tx output
    }
    else {
        handle_btc_output(context, page, &tx_out, &ret);
    }

    return ret;
}
static bool handle_qtum_output(app_context_t* context, tx_info_page_t* page, btc_tx_output_t* tx_out, int* ret) {
    const uint8_t* ptr = NULL;
    uint8_t* ptr1 = NULL;
    memset(page->to, 0, sizeof(page->to));
    memset(page->amount_str, 0, sizeof(page->amount_str));
    uint8_t len = btc_decode_qtum(tx_out->script, (uint32_t)tx_out->script_len, DATAHEX, &ptr);
    if (len == 0)
        return false;
    uint8_t data_len = len;

    len = btc_qtum_decode_token_data(ptr, data_len, QTUM_TO_ADDRESS, &ptr1);

    curr_info_t* curr = get_curr_info("QTUM");
    uint8_t net_id = ((uint8_t*)&curr->pub_key_hash)[3] == 0x01 ? ((uint8_t*)&curr->pub_key_hash)[0] : 0x3a;

    uint32_t address_len;
    to_base58_check(ptr1 + 12, 20, net_id, page->to, &address_len);
    page->to[address_len] = 0;
    len = btc_qtum_decode_token_data(ptr, data_len, QTUM_VALUE, &ptr1);

    uint8_t* num = NULL;
    len = remove_lead_zero(ptr1, len, &num);
    if (len == 0) {
        strcpy(page->amount_str, "0");
    }
    else {
        uint8_t tmp[len * 2];
        memset(tmp, 0, sizeof(tmp));
        len = uint8_extension(num, len, tmp, sizeof(tmp));
        len = base_convert(tmp, len, page->amount_str, sizeof(page->amount_str), 16, 10);
        num2str(page->amount_str, len);
    }
    return true;
}

static bool handle_tether_output(app_context_t* context, tx_info_page_t* page, btc_tx_output_t* tx_out, int* ret) {
    uint8_t tmp[10];
    memset(tmp, 0, sizeof(tmp));
    uint8_t len = btc_decode_tether(tx_out->script + 2, (uint32_t)tx_out->script_len, AMOUNT, tmp, sizeof(tmp));

    uint8_t buffer[len * 2];
    len = uint8_extension(tmp, len, buffer, sizeof(buffer));

    len = base_convert(buffer, len, page->amount_str, sizeof(page->amount_str), 16, 10);
    num2str(page->amount_str, len);

    int tx_len = (int)context->tx.tx_btc.output_len;

    for (size_t i = 0; i < tx_len; i++) {
        btc_tx_get_output(&context->tx.tx_btc, i, tx_out);
        if ((uint32_t)tx_out->value == 546) {
            uint32_t address_len;
            *ret = gen_btc_address(tx_out->script, (uint32_t)tx_out->script_len, context->curr->pub_key_hash, context->curr->script_hash, page->to, &address_len);
            if (*ret == 0)
                page->to[address_len] = 0;
            break;
        }
    }
    strcpy(page->title_str, "usdt");
}
static bool handle_btc_output(app_context_t* context, tx_info_page_t* page, btc_tx_output_t* tx_out, int* ret) {
    uint8_t amount[8];
    uint8_t buffer[16];
    write_uint64_be(amount, tx_out->value);
    uint8_t out_len = uint8_extension(amount, sizeof(amount), buffer, sizeof(buffer));
    out_len = base_convert(buffer, out_len, page->amount_str, sizeof(page->amount_str), 16, 10);
    num2str(page->amount_str, out_len);

    uint32_t address_len;
    *ret = gen_btc_address(tx_out->script, (uint32_t)tx_out->script_len, context->curr->pub_key_hash, context->curr->script_hash, page->to, &address_len);
    if (*ret == 0)
        page->to[address_len] = 0;
}

int btc_hash_256(void* param, uint8_t* data, size_t len, int step) {
    PRINTF("btc_hash_256: %.*h\n", len, data);
    app_context_t* context = (app_context_t*)param;
    if (step == STEP_BEGIN) {
        cx_sha256_init(&context->hash_context.hash.sha256);
    }

    if (len > 0) {
        cx_hash(&context->hash_context.hash.sha256.header, 0, data, len, NULL, 0);
    }

    if (step == STEP_END) {
        cx_hash(&context->hash_context.hash.sha256.header, CX_LAST, NULL, 0, context->hash_context.data.m32, 32);

        cx_sha256_init(&context->hash_context.hash.sha256);
        cx_hash(&context->hash_context.hash.sha256.header, CX_LAST, context->hash_context.data.m32, 32, context->hash_context.data.m32, 32);
    }

    return 0;
}

int btc_hash_256_for_sbtc(void* param, uint8_t* data, size_t len, int step) {
    if (step == STEP_END) {
        btc_hash_256(param, data, len, STEP_CONTINUOUS);
        uint8_t extra[] = {4, 's', 'b', 't', 'c'};
        return btc_hash_256(param, extra, 5, STEP_END);
    }
    return btc_hash_256(param, data, len, step);
}

int btc_hash_256_for_bifi(void* param, uint8_t* data, size_t len, int step) {
    if (step == STEP_VERSION) {
        app_context_t* context = (app_context_t*)param;
        os_memmove(data + 4, context->user_data + context->user_data_len - 4, 4);
        len += 4;
        return btc_hash_256(param, data, len, STEP_CONTINUOUS);
    }
    return btc_hash_256(param, data, len, step);
}

int btc_hash_256_for_bcd(void* param, uint8_t* data, size_t len, int step) {
    if (step == STEP_VERSION) {
        btc_hash_256(param, data, len, STEP_CONTINUOUS);
        app_context_t* context = (app_context_t*)param;
        os_memmove(data, context->user_data + context->user_data_len - 32, 32);
        len = 32;
        return btc_hash_256(param, data, len, STEP_CONTINUOUS);
    }
    return btc_hash_256(param, data, len, step);
}

int btc_sign_tx(app_context_t* context, uint32_t* path, uint8_t depth, uint32_t index, uint8_t* signature, int signature_capacity, int* signature_length) {
    // NOTE: user_data: 1-byte (hash type) + 4-byte (sig hash type) 1-byte (script length) + N-byte (script) + [8-byte (value)] + [others]
    uint8_t hashType = context->user_data[0];
    sighash_type_t st;
    read_uint32_le(context->user_data + 1, &st);
    uint8_t pre_out_script_len = context->user_data[1 + 4];
    uint8_t* pre_out_script = context->user_data + 1 + 4 + 1;
    PRINTF("btc_sign_tx: %d %.*h\n", st, 32, context->user_data);

    int ret = 1;

    if (hashType == 1) {  // hashForWitnessV0
        uint8_t all_hashes[64];
        ret = btc_hash_inputs(&context->tx.tx_btc, st, &btc_hash_256, (void*)context);
        os_memmove(all_hashes + 0, context->hash_context.data.m32, 32);
        ret = btc_hash_sequence(&context->tx.tx_btc, st, &btc_hash_256, (void*)context);
        os_memmove(all_hashes + 32, context->hash_context.data.m32, 32);
        ret = btc_hash_outputs(&context->tx.tx_btc, st, &btc_hash_256, (void*)context);

        uint64_t value;
        read_uint64_le(context->user_data + 1 + 4 + 1 + pre_out_script_len, &value);
        ret = btc_tx_hash_for_witness_v0(&context->tx.tx_btc, index, st, pre_out_script, pre_out_script_len, value, all_hashes + 0, all_hashes + 32, context->hash_context.data.m32, &btc_hash_256, (void*)context);
    }
    else if (hashType == 0) {  // hashForSignature
        if (!strcmp(context->coin->symbol, "SBTC")) {
            ret = btc_tx_hash_for_signature(&context->tx.tx_btc, index, st, pre_out_script, pre_out_script_len, &btc_hash_256_for_sbtc, (void*)context);
        }
        else if (!strcmp(context->coin->symbol, "BIFI")) {
            ret = btc_tx_hash_for_signature(&context->tx.tx_btc, index, st, pre_out_script, pre_out_script_len, &btc_hash_256_for_bifi, (void*)context);
        }
        else if (!strcmp(context->coin->symbol, "BCD")) {
            ret = btc_tx_hash_for_signature(&context->tx.tx_btc, index, st, pre_out_script, pre_out_script_len, &btc_hash_256_for_bcd, (void*)context);
        }
        else {
            ret = btc_tx_hash_for_signature(&context->tx.tx_btc, index, st, pre_out_script, pre_out_script_len, &btc_hash_256, (void*)context);
        }
    }

    if (ret != 0) {
        return ret;
    }

    PRINTF("btc_sign_tx: %.*h\n", 32, context->hash_context.data.m32);
    ret = sign(context->hash_context.data.m32, 32, signature, signature_capacity, signature_length, SECP256K1, path, depth, 0);
    return ret;
}

int btc_magic_hash(app_context_t* context) {
    const char* message_prefix = (const char*)PIC(context->curr->message_prefix);
    PRINTF("btc_magic_hash message_prefix: %s\n", message_prefix);
    uint8_t prefix_len = (uint8_t)strlen(message_prefix);
    btc_hash_256((void*)context, &prefix_len, 1, STEP_BEGIN);
    btc_hash_256((void*)context, message_prefix, prefix_len, STEP_CONTINUOUS);

    uint8_t buffer[10];
    int message_len = encode_var_int(buffer, context->tx_raw_len);
    btc_hash_256((void*)context, buffer, message_len, STEP_CONTINUOUS);
    btc_hash_256((void*)context, context->tx_raw, context->tx_raw_len, STEP_END);

    return 0;
}

int btc_sign_message(app_context_t* context, uint32_t* path, uint8_t depth, uint8_t* signature, int signature_capacity, int* signature_length) {
    int ret = 1;
    btc_magic_hash(context);
    PRINTF("btc_sign_message hash: %.*h\n", 32, context->hash_context.data.m32);
    ret = sign(context->hash_context.data.m32, 32, signature, signature_capacity, signature_length, SECP256K1, path, depth, 0);
    return ret;
}

static uint8_t btc_decode_tether(const uint8_t* src, uint32_t src_len, const tether_field_t index, uint8_t* out, uint8_t out_len) {
    if (index == PREFIX && out_len >= 4) {
        memcpy(out, src, 4);
        return 4;
    }
    else if (index == TX_TYPE && out_len >= 4) {
        memcpy(out, src + 4, 4);
        return 4;
    }
    else if (index == PROPERTY_ID && out_len >= 4) {
        memcpy(out, src + 8, 4);
        return 4;
    }
    else if (index == AMOUNT && out_len >= 8) {
        memcpy(out, src + 12, 8);
        return 8;
    }
    else {
        return 0;
    }
}

static uint8_t btc_decode_qtum(const uint8_t* buffer, uint32_t buffer_len, const qtum_field_t field_index, uint8_t** out) {
    uint8_t len = 0;
    uint8_t* start_ptr = buffer;
    qtum_field_t qtum_field_index = VM_VERSION;

    if (OP_CALL == field_index) {
        start_ptr = buffer + buffer_len - 1;
        len = 1;
        *out = start_ptr;
        return len;
    }

    for (uint32_t index = 0; index < buffer_len;) {
        len = start_ptr[index];
        if (qtum_field_index == field_index) {
            *out = start_ptr + index + 1;
            return len;
        }
        qtum_field_index++;
        index += len + 1;
    }
    return 0;
}

static uint8_t btc_qtum_decode_token_data(const uint8_t* buffer, uint32_t buffer_len, const qtum_transfer_token_data_t field_index, uint8_t** out) {
    uint8_t len = 0;
    if (field_index == QTUM_TRANSFER_HEX) {
        len = 4;
        *out = buffer;
    }
    else if (field_index == QTUM_TO_ADDRESS) {
        len = 32;
        *out = buffer + 4;
    }
    else if (field_index == QTUM_VALUE) {
        len = 32;
        *out = buffer + 32 + 4;
    }

    return len;
}