#ifndef ETH_M_TRANCACTION_H
#define ETH_M_TRANCACTION_H

#include "../common/rlp.h"
#include "../common/utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t* buffer;
    int buffer_len;
} eth_m_tx_t;

int eth_m_tx_init(eth_m_tx_t* tx, uint8_t* buffer, int buffer_len);
int eth_m_tx_update(eth_m_tx_t* tx);

uint8_t eth_m_tx_get_prefix(eth_m_tx_t* tx, uint8_t** out);
uint8_t eth_m_tx_get_address(eth_m_tx_t* tx, uint8_t** out);
uint8_t eth_m_tx_get_value(eth_m_tx_t* tx, uint8_t** out);
// uint8_t eth_m_tx_get_data(eth_m_tx_t* tx, uint8_t** out);

uint8_t eth_m_token_tx_get_prefix(eth_m_tx_t* tx, uint8_t** out);
uint8_t eth_m_token_tx_get_address(eth_m_tx_t* tx, uint8_t** out);
uint8_t eth_m_token_tx_get_value(eth_m_tx_t* tx, uint8_t** out);
uint8_t eth_m_token_tx_get_token_addr(eth_m_tx_t* tx, uint8_t** out);

#endif