#include "eos_transaction.h"
#include "../common/common.h"
#include "../common/utils.h"
#include "../common/tx_helper.h"

int read_varint32(uint8_t* buffer, uint32_t* out) {
    int c = 0;
    uint32_t value = 0;
    uint8_t b;
    do {
        b = buffer[c];
        if (c < 5) {
            value |= (b & 0x7f) << (7 * c);
        }
        c++;
    } while((b & 0x80) != 0);

    *out = value;
    return c;
}

int read_action(uint8_t* buffer, eos_tx_action_t* action) {
    int offset = 0;
    uint32_t size = 0;
    uint64_t u64;

    // account
    offset += read_uint64_le(buffer + offset, &u64);
    if (action) {
        action->account = u64;
    }

    // name
    offset += read_uint64_le(buffer + offset, &u64);
    if (action) {
        action->name = u64;
    }

    // authorization
    offset += read_varint32(buffer + offset, &size);
    if (action) {
        action->authorization_index = offset;
        action->authorization_len = size;
    }
    
    for (int i = 0; i < size; i++) {
        // actor
        offset += read_uint64_le(buffer + offset, &u64);

        // permission
        offset += read_uint64_le(buffer + offset, &u64);
    }

    // data
    offset += read_varint32(buffer + offset, &size);
    if (action) {
        action->data = buffer + offset;
        action->data_len = size;
    }
    offset += size;

    return offset;
}

int read_action_transfer(uint8_t* buffer, eos_tx_action_transfer_t* transfer) {
    int offset = 0;

    // from
    uint64_t from;
    offset += read_uint64_le(buffer + offset, &from);
    if (transfer) {
        transfer->from = from;
    }

    // to
    uint64_t to;
    offset += read_uint64_le(buffer + offset, &to);
    if (transfer) {
        transfer->to = to;
    }

    // quantity
    int64_t amount;
    uint64_t symbol;
    offset += read_uint64_le(buffer + offset, &amount);
    offset += read_uint64_le(buffer + offset, &symbol);
    if (transfer) {
        transfer->amount = amount;
        transfer->symbol = symbol;
    }

    // memo
    uint32_t size = 0;
    offset += read_varint32(buffer + offset, &size);
    if (transfer) {
        transfer->memo = buffer + offset;
        transfer->memo_len = size;
    }
    offset += size;

    return offset;
}

int read_extension(uint8_t* buffer, eos_tx_extension_t* extension) {
    int offset = 0;

    // type
    uint16_t type;
    offset += read_uint16_le(buffer + offset, &type);
    if (extension) {
        extension->type = type;
    }

    // data
    uint32_t size;
    offset += read_varint32(buffer + offset, &size);
    if (extension) {
        extension->data = buffer + offset;
        extension->data_len = size;
    }
    offset += size;

    return offset;
}

int eos_tx_init(eos_tx_t* tx, uint8_t* buffer, int buffer_len) {
    if (!tx || !buffer || buffer_len <= 0) {
        return 1;
    }
    tx->buffer = buffer;
    tx->buffer_len = buffer_len;
    return eos_tx_update(tx);
}

int eos_tx_update(eos_tx_t* tx) {
    tx->tx_raw_index = 32;
    tx->tx_raw_len = tx->buffer_len - 32 - 32;

    int16_t tx_raw_end = tx->tx_raw_len + tx->tx_raw_index;
    int16_t offset = tx->tx_raw_index;
    uint32_t temp = 0;

    // expiration
    offset += 4;

    // ref_block_num
    offset += 2;

    // ref_block_prefix
    offset += 4;

    // net_usage_words
    if (tx_raw_end < offset + 1) return 1;
    offset += read_varint32(tx->buffer + offset, &temp);

    // max_cpu_usage_ms
    offset += 1;

    // delay_sec
    if (tx_raw_end < offset + 1) return 1;
    offset += read_varint32(tx->buffer + offset, &temp);

    // context_free_actions
    if (tx_raw_end < offset + 1) return 1;
    offset += read_varint32(tx->buffer + offset, &temp);
    for (int i = 0; i < temp; i++) {
        if (tx_raw_end < offset + 1) return 1;
        offset += read_action(tx->buffer + offset, NULL);
    }

    // actions
    if (tx_raw_end < offset + 1) return 1;
    offset += read_varint32(tx->buffer + offset, &temp);
    tx->action_index = offset;
    tx->action_len = temp;
    for (int i = 0; i < temp; i++) {
        if (tx_raw_end < offset + 1) return 1;
        offset += read_action(tx->buffer + offset, NULL);
    }

    // transaction_extensions
    if (tx_raw_end < offset + 1) return 1;
    offset += read_varint32(tx->buffer + offset, &temp);
    for (int i = 0; i < temp; i++) {
        if (tx_raw_end < offset + 1) return 1;
        offset += read_extension(tx->buffer + offset, NULL);
    }

    return 0;
}

int eos_tx_get_action(eos_tx_t* tx, uint32_t index, eos_tx_action_t* action) {
    int offset = tx->action_index;
    for (int i = 0; i < tx->action_len; i++) {
        offset += read_action(tx->buffer + offset, i == index ? action : NULL);
        if (i == index) {
            return 0;
        }
    }
    return 1;
}

int eos_tx_get_action_transfer(eos_tx_action_t* action, eos_tx_action_transfer_t* transfer) {
    read_action_transfer(action->data, transfer);
    return 0;
}

int eos_decode_name(uint64_t name, uint8_t* out, uint32_t * out_len) {
    char char_map[] = ".12345abcdefghijklmnopqrstuvwxyz";
    const uint8_t MAX_LEN = 12;
    uint8_t i;
    for (i = 0; i <= MAX_LEN; i++) {
        out[MAX_LEN - i] = char_map[name & (i ? 0x1f : 0x0f)];
        name = name >> (i ? 5 : 4);
    }
    for (i = MAX_LEN; i >= 0; i--) {
        if (out[i] == '.') {
            out[i] = 0;
        }
        else {
            break;
        }
    }
    *out_len = i + 1;
    return 0;
}

int eos_decode_symbol(uint64_t symbol, uint8_t* out, uint32_t * out_len) {
    uint8_t len = (symbol & 0xff) - 1;
    for (int i = 0; i < len; i++) {
        symbol = symbol >> 8;
        out[i] = symbol & 0xff;
    }
    *out_len = len;
    return 0;
}

int eos_tx_hash_for_signature(eos_tx_t* tx, hash_func_t hasher, void* param) {
    uint8_t output[32];
    int offset = 0;
    int ret;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_BEGIN);

    for (int j = 0; j < tx->buffer_len; j++) {
        // [output hash]
        if (offset >= 32) {
            HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
        }

        output[offset++] = tx->buffer[j];
    }

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_END);

    return 0;
}