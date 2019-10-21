#pragma once

#include <stdint.h>

void tx_buffer_initialize();

void tx_buffer_reset();

uint32_t tx_buffer_append(uint8_t *buffer, uint32_t length);

uint32_t tx_buffer_get_buffer_length();

uint8_t *tx_buffer_get_buffer();