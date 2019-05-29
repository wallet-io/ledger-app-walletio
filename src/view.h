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
#pragma once

#include <stdbool.h>
#include "os.h"
#include "cx.h"
#include "view_common.h"
#include "view_expl.h"
#include "view_conf.h"

enum UI_STATE {
    UI_IDLE,
    UI_TRANSACTION
};

extern enum UI_STATE view_uiState;

//------ Event handlers
/// view_set_handlers
void view_set_handlers(viewctl_delegate_getData func_getData,
                       viewctl_delegate_accept func_accept,
                       viewctl_delegate_reject func_reject);

//------ Common functions
/// view_init
void view_init(void);

/// view_idle
void view_idle(unsigned int ignored);

/// view_sign_confirm
void view_sign_confirm(unsigned int start_page);

/// view_unlock_confirm
void view_unlock_confirm(unsigned int start_page);

