#include "base58.h"
#include "common.h"

#define MAX_DEC_INPUT_SIZE 164
#define MAX_ENC_INPUT_SIZE 120

static uint8_t const BASE58_TABLE[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,  0x7,  0x8,  0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x9,  0xa,  0xb,  0xc,  0xd,  0xe,  0xf,
    0x10, 0xff, 0x11, 0x12, 0x13, 0x14, 0x15, 0xff, 0x16, 0x17, 0x18, 0x19,
    0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
    0xff, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0xff, 0xff, 0xff, 0xff, 0xff};

static uint8_t const BASE58_ALPHABET[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

int decode_base58(const char *in, size_t length, uint8_t *out, size_t out_len) {
    uint8_t tmp[MAX_DEC_INPUT_SIZE];
    uint8_t buffer[MAX_DEC_INPUT_SIZE] = {0};
    uint8_t i, j, start_at;
    uint8_t zero_count = 0;
    if (length > MAX_DEC_INPUT_SIZE) {
        return -1;
    }

    MEMMOVE(tmp, in, length);

    for (i = 0; i < length; i++) {
        if (in[i] >= sizeof(BASE58_TABLE)) {
            return -1;
        }
        tmp[i] = BASE58_TABLE[(int)in[i]];
        if (tmp[i] == 0xff) {
            return -1;
        }
    }

    while ((zero_count < length) && (tmp[zero_count] == 0)) {
        ++zero_count;
    }

    j = length;
    start_at = zero_count;
    while (start_at < length) {
        unsigned short remainder = 0;
        uint8_t loop;
        for (loop = start_at; loop < length; loop++) {
            unsigned short digit256 = (unsigned short)(tmp[loop] & 0xff);
            unsigned short temp = remainder * 58 + digit256;
            tmp[loop] = (uint8_t)(temp / 256);
            remainder = (temp % 256);
        }
        if (tmp[start_at] == 0) {
            ++start_at;
        }
        buffer[--j] = (uint8_t)remainder;
    }
    while ((j < length) && (buffer[j] == 0)) {
        ++j;
    }
    length = length - (j - zero_count);
    if (out_len < length) {
        return -1;
    }

    return length;
}

int encode_base58(const uint8_t *in, size_t in_len, uint8_t *out, size_t out_len) {
    uint8_t buffer[MAX_ENC_INPUT_SIZE * 138 / 100 + 1] = {0};
    size_t i = 0, j;
    size_t start_at, stop_at;
    size_t zero_count = 0;
    size_t output_size;

    if (in_len > MAX_ENC_INPUT_SIZE) {
        return -1;
    }

    while ((zero_count < in_len) && (in[zero_count] == 0)) {
        ++zero_count;
    }

    output_size = (in_len - zero_count) * 138 / 100 + 1;
    stop_at = output_size - 1;
    for (start_at = zero_count; start_at < in_len; start_at++) {
        int carry = in[start_at];
        for (j = output_size - 1; (int)j >= 0; j--) {
            carry += 256 * buffer[j];
            buffer[j] = carry % 58;
            carry /= 58;

            if (j <= stop_at - 1 && carry == 0) {
                break;
            }
        }
        stop_at = j;
    }

    j = 0;
    while (j < output_size && buffer[j] == 0) {
        j += 1;
    }

    if (out_len < zero_count + output_size - j) {
        out_len = zero_count + output_size - j;
        return -1;
    }

    MEMSET(out, BASE58_ALPHABET[0], zero_count);

    i = zero_count;
    while (j < output_size) {
        out[i++] = BASE58_ALPHABET[buffer[j++]];
    }
    return i;
}
