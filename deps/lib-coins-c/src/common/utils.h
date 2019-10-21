#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int32_t read_uint16_le(const uint8_t * buffer, uint16_t * out);
int32_t read_uint32_le(const uint8_t * buffer, uint32_t * out);
int32_t read_uint64_le(const uint8_t * buffer, uint64_t * out);

int32_t read_uint16_be(const uint8_t * buffer, uint16_t * out);
int32_t read_uint32_be(const uint8_t * buffer, uint32_t * out);
int32_t read_uint64_be(const uint8_t * buffer, uint64_t * out);

int32_t write_uint16_le(uint8_t * buffer, uint16_t value);
int32_t write_uint32_le(uint8_t * buffer, uint32_t value);
int32_t write_uint64_le(uint8_t * buffer, uint64_t value);

int32_t write_uint16_be(uint8_t * buffer, uint16_t value);
int32_t write_uint32_be(uint8_t * buffer, uint32_t value);
int32_t write_uint64_be(uint8_t * buffer, uint64_t value);

int32_t decode_var_int(const uint8_t * buffer, int64_t * out);
int32_t encode_var_int(uint8_t * buffer, int64_t value);

int buffer_to_hex(const uint8_t * buffer, size_t buffer_len, uint8_t * out, size_t out_len);
int hex_to_buffer(const uint8_t * hex, size_t hex_len, uint8_t * out, size_t out_len);
uint8_t ch_to_num(uint8_t ch);
uint32_t hex_to_buf(uint8_t* in, uint32_t in_len, uint8_t* out, uint32_t out_capacity);
typedef int (*hash_func_t)(void * param, uint8_t * data, size_t len, int step);
#endif
