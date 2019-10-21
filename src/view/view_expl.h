#pragma once

#include "os.h"
#include "cx.h"
#include "view_common.h"
#include <stdbool.h>
// Initialize and show control
void viewexpl_start(int start_page, bool single_page,
                    viewctl_delegate_getData delegate_update,
                    viewctl_delegate_ready delegate_ready,
                    viewctl_delegate_exit delegate_exit);
