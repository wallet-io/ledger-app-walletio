#pragma once

#include "app_context.h"

#include <stdint.h>

enum UI_STATE { UI_IDLE, UI_TRANSACTION };

//------ Common functions
// view_init
void view_init(void);

// view_idle
void view_idle(unsigned int ignored);

// view_tx_show
void view_tx_show(app_context_t *con);
