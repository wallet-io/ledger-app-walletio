#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool platform_is_big_endian();
uint8_t base_convert(const uint8_t *buffer, const uint8_t buffer_len, uint8_t *out, const uint8_t out_capcity, const uint8_t old_base, const uint8_t new_base);
uint8_t uint8_extension(const uint8_t *src, const uint8_t src_len, uint8_t *out, uint8_t out_capcity);
bool num2str(uint8_t *src, uint8_t src_len);
bool str_cut(const uint8_t *src, uint8_t src_len, uint8_t *out, uint8_t out_len, uint8_t style);
uint8_t remove_lead_zero(uint8_t *src, uint8_t src_len, uint8_t **out);
#endif