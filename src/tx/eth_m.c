#include "../app_context.h"
#include "btc.h"
#include "crypto.h"

#include "common/tx_helper.h"
#include "common/utils.h"
#include "eth_m.h"
#include "eth_m/eth_m_transaction.h"
#include "os.h"
#include "tx.h"

static uint8_t global_tmp[68];

int eth_m_init_tx(app_context_t* context) {
    return eth_m_tx_init(&context->tx.tx_eth_m, context->tx_raw, context->tx_raw_len);
}

int eth_m_get_tx_info_page_size(app_context_t* context) {
    return 1;
}

static int handle_tx(app_context_t* context, uint32_t index, tx_info_page_t* page) {
    eth_m_tx_t* tx = &context->tx.tx_eth;
    uint8_t* address = NULL;
    uint8_t* value = NULL;

    uint8_t len = eth_m_tx_get_address(tx, &address);
    memcpy(page->to, "0x", 2);
    len = buffer_to_hex(address, len, page->to + 2, sizeof(page->to) - 2);

    len = eth_m_tx_get_value(tx, &value);
    len = remove_lead_zero(value, len, &value);
    uint8_t temp1[len * 2];
    memset(temp1, 0, sizeof(temp1));
    len = uint8_extension(value, len, temp1, sizeof(temp1));
    len = base_convert(temp1, len, page->amount_str, sizeof(page->amount_str), 16, 10);
    num2str(page->amount_str, len);
    return 0;
}

static int handle_token_tx(app_context_t* context, uint32_t index, tx_info_page_t* page) {
    eth_m_tx_t* tx = &context->tx.tx_eth;
    uint8_t* address = NULL;
    uint8_t* value = NULL;

    uint8_t len = eth_m_token_tx_get_address(tx, &address);
    memcpy(page->to, "0x", 2);
    len = buffer_to_hex(address, len, page->to + 2, sizeof(page->to) - 2);

    len = eth_m_token_tx_get_token_addr(tx, &address);
    memcpy(page->eth_token_addr_str, "0x", 2);
    len = buffer_to_hex(address, len, page->eth_token_addr_str + 2, sizeof(page->eth_token_addr_str) - 2);
    
    len = eth_m_token_tx_get_value(tx, &value);
    len = remove_lead_zero(value, len, &value);
    uint8_t temp1[len * 2];
    memset(temp1, 0, sizeof(temp1));
    len = uint8_extension(value, len, temp1, sizeof(temp1));
    len = base_convert(temp1, len, page->amount_str, sizeof(page->amount_str), 16, 10);
    num2str(page->amount_str, len);
    return 0;
}

int eth_m_get_tx_info_page(app_context_t* context, uint32_t index, tx_info_page_t* page) {
    uint8_t* prefix = NULL;
    eth_m_token_tx_get_prefix(&context->tx.tx_eth_m, &prefix);
    if (strncmp(prefix, "ETHER", 5) == 0)
        return handle_tx(context, index, page);
    if (strncmp(prefix, "ERC20", 5) == 0)
        return handle_token_tx(context, index, page);
}