#ifndef __RLP_H__
#define __RLP_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int rlp_get_array_length(uint8_t * buffer, size_t buffer_len, uint32_t * length);

int rlp_get_array_item(uint8_t * buffer, size_t buffer_len, uint32_t index, uint8_t * out, size_t * out_len, size_t out_cap);

#endif
