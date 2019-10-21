#include "view.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "address.h"
#include "apdu_codes.h"
#include "app_context.h"
#include "bagl.h"
#include "common.h"
#include "common/utils.h"
#include "eth/eth_transaction.h"
#include "glyphs.h"
#include "view_conf.h"
#include "view_expl.h"
#include "view_templates.h"
#include "tx/eth_m.h"

#include "tx/tx.h"

#define TRUE 1
#define FALSE 0

ux_state_t ux;
static enum UI_STATE view_uiState;
static app_context_t *context;

extern int viewctl_accept_page_index;
extern int viewctl_reject_page_index;

//------ View elements
const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

const ux_menu_entry_t menu_main[] = {
    {NULL, NULL, 0, &C_icon_app, "Ledger Signer", "Ready", 31, 8},
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END};

const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
    {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

//------ View elements

//------ Event handlers
viewctl_delegate_getData ehGetData = NULL;
viewctl_delegate_accept ehAccept = NULL;
viewctl_delegate_reject ehReject = NULL;

void view_set_handlers(viewctl_delegate_getData func_getData,
                       viewctl_delegate_accept func_accept,
                       viewctl_delegate_reject func_reject) {
    ehGetData = func_getData;
    ehAccept = func_accept;
    ehReject = func_reject;
}

// ------ Event handlers

void io_seproxyhal_display(const bagl_element_t *element) {
    io_seproxyhal_display_default((bagl_element_t *)element);
}

void view_init(void) {
    UX_INIT();
    view_uiState = UI_IDLE;
}

void view_idle(unsigned int ignored) {
    PRINTF("view_idle\n");
    view_uiState = UI_IDLE;
    UX_MENU_DISPLAY(0, menu_main, NULL);
}

int tx_info_get_data(char *title, int max_title_length, char *key,
                     int max_key_length, char *value, int max_value_length,
                     int page_index, int chunk_index, int *page_count_out,
                     int *chunk_count_out) {
    if (page_index == viewctl_accept_page_index) {
        snprintf(title, max_title_length, "");
        strcpy(key, "accept?");
        strcpy(value, "press both buttons to select");
        return 0;
    }

    if (page_index == viewctl_reject_page_index) {
        snprintf(title, max_title_length, "");
        strcpy(key, "reject?");
        strcpy(value, "press both buttons to select");
        return 0;
    }

    // tx_info_page_t page;
    memset(&context->page, 0, sizeof(context->page));
    int ret = get_tx_info_page(context, context->signing_type == SIGN_TX ? page_index / 2 : page_index, &context->page);
    if (ret != 0) {
        strcpy(context->page.to, "ERROR");
        strcpy(context->page.amount_str, "0");
    }

    if (context->signing_type == SIGN_MESSAGE) {
        snprintf(key, max_key_length, "coin:%s", context->page.amount_str);
        snprintf(value, max_value_length, "msg: %s", context->page.to);
    }
    else if (context->signing_type == SIGN_TX) {
        uint8_t* prefix = NULL;
        eth_m_token_tx_get_prefix(&context->tx.tx_eth_m, &prefix);
        if (strcmp(context->coin->cate, "ETHM") == 0 && strncmp(prefix, "ERC20", 5) == 0) {
            if (page_index == 1) {
                snprintf(key, max_key_length, "%s", context->page.to);
                snprintf(value, max_value_length, "%s", context->page.amount_str);
            }
            else {
                snprintf(key, max_key_length, "%s", "token address");
                snprintf(value, max_value_length, "%s", context->page.eth_token_addr_str);
            }
        }
        else {
            snprintf(key, max_key_length, "%s", page_index % 2 ? "amount" : "to");
            snprintf(value, max_value_length, "%s", page_index % 2 ? context->page.amount_str : context->page.to);
        }
    }

    snprintf(title, max_title_length, "%s %d/%d", strlen(context->page.title_str) != 0 ? context->page.title_str : "", page_index + 1, *page_count_out - 2);
    
    PRINTF("title:%s\n", title);
    PRINTF("key:%s\n", key);
    PRINTF("value:%s\n", value);
    return 0;
}
void tx_info_accept(unsigned int unused) {
    PRINTF("tx_info_accept\n");
    context->accept_signing = true;
    G_io_apdu_buffer[0] = context->accept_signing;
    set_code(G_io_apdu_buffer, 1, APDU_CODE_OK);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 1 + 2);
    view_idle(0);
}
void tx_info_reject(unsigned int unused) {
    PRINTF("tx_info_reject\n");
    context->accept_signing = false;
    G_io_apdu_buffer[0] = context->accept_signing;
    set_code(G_io_apdu_buffer, 1, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 1 + 2);
    view_idle(0);
}

void view_tx_show(app_context_t *con) {
    PRINTF("view_tx_show\n");
    context = con;

    int page_size = get_tx_info_page_size(context);
    viewctl_DetailsPageCount = page_size + 2;
    viewctl_ChunksCount = 1;
    viewctl_accept_page_index = viewctl_DetailsPageCount - 2;
    viewctl_reject_page_index = viewctl_DetailsPageCount - 1;

    view_set_handlers(tx_info_get_data, NULL, NULL);
    // the default exit exec tx_info_reject
    viewexpl_start(0, false, ehGetData, NULL, tx_info_reject);
}
