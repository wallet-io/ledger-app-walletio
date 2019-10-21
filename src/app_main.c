#include "app_main.h"
#include "apdu_handler.h"
#include "app_context.h"
#include "view/view.h"
#include "zxmacros.h"

#include <os.h>
#include <os_io_seproxyhal.h>

#include <string.h>

app_context_t context;

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

unsigned char io_event(unsigned char channel) {
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_FINGER_EVENT:  //
            UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:  // for Nano S
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
            if (!UX_DISPLAYED()) UX_DISPLAYED_EVENT();
            break;

        case SEPROXYHAL_TAG_TICKER_EVENT: {  //
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
                if (UX_ALLOWED) {
                    UX_REDISPLAY();
                }
            });
            break;
        }

            // unknown events are acknowledged
        default:
            UX_DEFAULT_EVENT();
            break;
    }
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }
    return 1;  // DO NOT reset the current APDU transport
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    PRINTF("io_exchange_al 1: %p %p %p\n", channel, IO_FLAGS, channel & ~(IO_FLAGS));
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;

            // multiplexed io exchange over a SPI channel and TLV encapsulated
            // protocol
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0;  // nothing received from the master so far (it's a tx
                // transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                              sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(INVALID_PARAMETER);
    }
    return 0;
}

void app_init() {
    io_seproxyhal_init();
    USB_power(0);
    USB_power(1);
    view_idle(0);

    app_context_reset(&context);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void app_main() {
    volatile uint32_t rx = 0, tx = 0, flags = 0;

    for (;;) {
        volatile uint16_t sw = 0;

        BEGIN_TRY;
        {
            TRY;
            {
                rx = tx;
                tx = 0;
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                if (rx == 0) THROW(APDU_CODE_EMPTY_BUFFER);

                handle_apdu(&context, &flags, &tx, rx);
            }
            CATCH_OTHER(e);
            {
                switch (e & 0xF000) {
                    case 0x6000:
                    case 0x9000:
                        sw = e;
                        break;
                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY;
            {}
        }
        END_TRY;
    }
}

#pragma clang diagnostic pop
