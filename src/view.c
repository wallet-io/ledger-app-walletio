/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2019 WalletIO
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include "view.h"
#include "view_templates.h"
#include "view_expl.h"
#include "view_conf.h"
#include "common.h"
#include "app_main.h"

#include "glyphs.h"
#include "bagl.h"

#include <string.h>
#include <stdio.h>

#define TRUE  1
#define FALSE 0

ux_state_t ux;
enum UI_STATE view_uiState;

void view_reject(unsigned int unused);

//------ View elements
const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_about[];

const ux_menu_entry_t menu_main[] = {
    {NULL, NULL, 0, &C_icon_app, "Ledger Signer", "Ready", 31, 8},
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END
};

const ux_menu_entry_t menu_about[] = {
        {NULL, NULL, 0, NULL, "Version", APPVERSION, 0, 0},
        {menu_main, NULL, 2, &C_icon_back, "Back", NULL, 61, 40},
        UX_MENU_END
};
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
    io_seproxyhal_display_default((bagl_element_t *) element);
}

void view_init(void) {
    UX_INIT();
    view_uiState = UI_IDLE;
}

void view_idle(unsigned int ignored) {
    view_uiState = UI_IDLE;
    UX_MENU_DISPLAY(0, menu_main, NULL);
}

void view_sign_confirm(unsigned int start_page) {
    if (ehGetData == NULL) { return; }
    viewconf_start(start_page,
                   true,        // single page
                   ehGetData,   // update
                   NULL,        // ready
                   NULL,        // exit
                   ehAccept,
                   ehReject);
}

void view_unlock_confirm(unsigned int start_page) {
    if (ehGetData == NULL) { return; }
    viewconf_start(start_page,
                   true,        // single page
                   ehGetData,   // update
                   NULL,        // ready
                   NULL,        // exit
                   ehAccept,
                   ehReject);
}

