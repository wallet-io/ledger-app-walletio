#pragma once

#include "cx.h"
#include "os.h"
#include "view/view_common.h"
// Initialize and show control
void viewconf_start(int start_page, bool single_page,
                    viewctl_delegate_getData func_update,
                    viewctl_delegate_ready func_ready,
                    viewctl_delegate_exit func_exit,
                    viewctl_delegate_accept func_accept,
                    viewctl_delegate_reject func_reject);
