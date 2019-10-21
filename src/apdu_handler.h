#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "apdu_codes.h"
#include "app_context.h"
#include "common.h"

#define CLA 0xBC

#define OFFSET_CLA 0
#define OFFSET_INS 1        //< Instruction offset
#define OFFSET_PCK_INDEX 2  //< Package index offset
#define OFFSET_PCK_COUNT 3  //< Package count offset
#define OFFSET_DATA 5       //< Data offset

#define INS_GET_VERSION 0
#define INS_PUBLIC_KEY 1
#define INS_SIGN_TX 2
#define INS_REQUEST_SIG 3
#define INS_SIGN_MESSAGE 4

#define MAX_BIP32_DEPTH 5

void handle_apdu(app_context_t *context, volatile uint32_t *flags,
                 volatile uint32_t *tx, uint32_t rx);
