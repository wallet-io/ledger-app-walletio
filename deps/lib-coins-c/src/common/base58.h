#ifndef BASE58_H
#define BASE58_H

#include <stdint.h>
#include <stdlib.h>

int decode_base58(const char *in, size_t length, uint8_t *out, size_t out_len);

int encode_base58(const uint8_t *in, size_t length, uint8_t *out, size_t out_len);

#endif