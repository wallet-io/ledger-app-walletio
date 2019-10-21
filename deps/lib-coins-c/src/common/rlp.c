#include "rlp.h"
#include "../common/utils.h"

typedef struct {
    uint32_t data_offset;
    uint32_t data_length;
    uint32_t remainder_offset;
    uint32_t remainder_length;
    uint8_t type;
} decode_result_t;

int read_rlp_length(uint8_t * buffer, size_t buffer_len, int32_t * out) {
    if (!buffer || buffer_len < 0 || buffer_len > 4) {
        return 1;
    }
    *out = 0;
    // rlp encoded length as big-endian
    for (int i = buffer_len - 1; i >= 0; i--) {
        ((int8_t*)out)[0] = buffer[i];
    }
    return 0;
}

int rlp_decode(uint8_t * buffer, size_t buffer_len, decode_result_t * result) {
    if (!buffer || buffer_len <= 0) {
        return 1;
    }

    uint8_t first = buffer[0];
    uint32_t length;
    if (first <= 0x7f) {
        length = 1;
        result->data_offset = 0;
        result->data_length = length;
        result->remainder_offset = length;
        result->remainder_length = buffer_len - length;
        result->type = 0;
    }
    else if (first <= 0xb7) {
        length = first - 0x7f;
        result->data_offset = 1;
        result->data_length = length - 1;
        result->remainder_offset = length;
        result->remainder_length = buffer_len - length;
        result->type = 0;
    }
    else if (first <= 0xbf) {
        length = first - 0xb6;
        int32_t data_length;
        if (read_rlp_length(buffer + 1, length - 1, &data_length) != 0) {
            return 1;
        }

        result->data_offset = length;
        result->data_length = data_length;
        result->remainder_offset = length + data_length;
        result->remainder_length = buffer_len - length - data_length;
        result->type = 0;
    }
    else if (first <= 0xf7) {
        length = first - 0xbf;
        result->data_offset = 1;
        result->data_length = length - 1;
        result->remainder_offset = length;
        result->remainder_length = buffer_len - length;
        result->type = 1;
    }
    else {
        length = first - 0xf6;
        int32_t data_length;
        if (read_rlp_length(buffer + 1, length - 1, &data_length) != 0) {
            return 1;
        }

        result->data_offset = length;
        result->data_length = data_length;
        result->remainder_offset = length + data_length;
        result->remainder_length = buffer_len - length - data_length;
        result->type = 1;
    }

    return 0;
}

int rlp_get_array_length(uint8_t * buffer, size_t buffer_len, uint32_t * out) {
    decode_result_t result;
    uint8_t * start = buffer;
    uint32_t remainder = buffer_len;
    int count = 0;

    if (rlp_decode(start, remainder, &result) != 0) {
        return 1;
    }

    if (!result.type) { // it's just a raw type not list
        return 1;
    }

    start = start + result.data_offset;
    remainder = result.data_length;
    while (remainder > 0) {
        if (rlp_decode(start, remainder, &result) != 0) {
            return 1;
        }
        
        start = start + result.remainder_offset;
        remainder = result.remainder_length;

        count++;
    }

    *out = count;
    return 0;
}

int rlp_get_array_item(uint8_t * buffer, size_t buffer_len, uint32_t index, uint8_t * out, size_t * out_len, size_t out_cap) {
    decode_result_t result;
    uint8_t * start = buffer;
    uint32_t remainder = buffer_len;
    int count = 0;

    if (rlp_decode(start, remainder, &result) != 0) {
        return 1;
    }

    if (!result.type) { // it's just a raw type not list
        return 1;
    }

    start = start + result.data_offset;
    remainder = result.data_length;
    while (remainder > 0) {
        if (rlp_decode(start, remainder, &result) != 0) {
            return 1;
        }

        if (index == count) {
            *out_len = result.data_length;
            for (int i = 0; i < result.data_length && i < out_cap; i++) {
                out[i] = start[result.data_offset + i];
            }
            break;
        }

        start = start + result.remainder_offset;
        remainder = result.remainder_length;

        count++;
    }

    return 0;
}
