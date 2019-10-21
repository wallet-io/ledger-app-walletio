#pragma once

#include <stdint.h>

int gen_btc_address(uint8_t* script, uint32_t script_len, uint32_t pub_key_hash,
                    uint32_t script_hash, uint8_t* address,
                    uint32_t* address_len);
