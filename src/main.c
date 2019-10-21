#include "app_main.h"
#include "view/view.h"

#include <os_io_seproxyhal.h>

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    view_init();
    os_boot();

    BEGIN_TRY {
        TRY {
            app_init();
            app_main();
        }
        CATCH_OTHER(e) {}
        FINALLY {}
    }
    END_TRY;
}
