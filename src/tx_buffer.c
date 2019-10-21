#include "tx_buffer.h"
#include "buffering.h"
#include "os.h"

// Ram
#define RAM_BUFFER_SIZE 500
uint8_t ram_buffer[RAM_BUFFER_SIZE];

// Flash
#define FLASH_BUFFER_SIZE 10000
typedef struct {
    uint8_t buffer[FLASH_BUFFER_SIZE];
} storage_t;

storage_t N_appdata_impl __attribute__((aligned(64)));
#define N_appdata (*(storage_t *)PIC(&N_appdata_impl))

void update_ram(buffer_state_t *buffer, uint8_t *data, int size) {
    os_memmove(buffer->data + buffer->pos, data, size);
}

void update_flash(buffer_state_t *buffer, uint8_t *data, int size) {
    nvm_write((void *)buffer->data + buffer->pos, data, size);
}

void tx_buffer_initialize() {
    append_buffer_delegate update_ram_delegate = &update_ram;
    append_buffer_delegate update_flash_delegate = &update_flash;

    buffering_init(ram_buffer, sizeof(ram_buffer), update_ram_delegate,
                   N_appdata.buffer, sizeof(N_appdata.buffer),
                   update_flash_delegate);
}

void tx_buffer_reset() { buffering_reset(); }

uint32_t tx_buffer_append(uint8_t *buffer, uint32_t length) {
    return buffering_append(buffer, length);
}

uint32_t tx_buffer_get_buffer_length() { return buffering_get_buffer()->pos; }

uint8_t *tx_buffer_get_buffer() { return buffering_get_buffer()->data; }
