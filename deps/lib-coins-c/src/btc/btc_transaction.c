#include "btc_transaction.h"
#include "../common/utils.h"
#include "../common/tx_helper.h"

#define DEFAULT_SEQUENCE 0xffffffff

#define ADVANCED_TRANSACTION_MARKER 0x00
#define ADVANCED_TRANSACTION_FLAG 0x01

int btc_tx_init(btc_tx_t* tx, uint8_t* buffer, int buffer_len) {
    if (!tx || !buffer || buffer_len <= 0) {
        return 1;
    }

    tx->buffer = buffer;
    tx->buffer_len = buffer_len;
    return btc_tx_update(tx);
}

int btc_tx_update(btc_tx_t* tx) {
    if (!tx || !tx->buffer || tx->buffer_len <= 0) {
        return 1;
    }
    int32_t offset = 0;

    // version
    if (tx->buffer_len < offset + 4) return 1;
    offset += read_uint32_le(tx->buffer, &tx->version);

    if (tx->mask & BTC_TX_MASK_NTIME) {
        // ntime
        if (tx->buffer_len < offset + 4) return 1;
        offset += 4;
    }

    // witness marker & flag
    if (tx->buffer_len < offset + 2) return 1;
    uint8_t marker = (uint8_t)tx->buffer[offset];
    uint8_t flag = (uint8_t)tx->buffer[offset + 1];
    if (marker == ADVANCED_TRANSACTION_MARKER && flag == ADVANCED_TRANSACTION_FLAG) {
        offset += 2;
        tx->has_witnesses = true;
    }

    // inputs
    offset += decode_var_int(tx->buffer + offset, &tx->input_len);
    tx->input_index = offset;
    int script_len;
    for (int i = 0; i < tx->input_len; i++) {
        offset += 32; // hash
        offset += 4;  // index
        // script
        offset += decode_var_int(tx->buffer + offset, &script_len);
        offset += script_len;
        offset += 4;  // sequence
        if (tx->buffer_len < offset) return 1;
    }

    // outputs
    offset += decode_var_int(tx->buffer + offset, &tx->output_len);
    tx->output_index = offset;
    for (int i = 0; i < tx->output_len; i++) {
        offset += 8;  // value
        // script
        offset += decode_var_int(tx->buffer + offset, &script_len);
        offset += script_len;
        if (tx->buffer_len < offset) return 1;
    }

    // witness
    if (tx->has_witnesses) {
        tx->witnesses_index = offset;
        int vector_len;
        for (int i = 0; i < tx->input_len; i++) {
            offset += decode_var_int(tx->buffer + offset, &vector_len);
            offset += vector_len;
            for (int j = 0; j < vector_len; j++) {
                offset += decode_var_int(tx->buffer + offset, &script_len);
                offset += script_len;
                if (tx->buffer_len < offset) return 1;
            }
            if (tx->buffer_len < offset) return 1;
        }
        tx->witnesses_len = offset - tx->witnesses_index;
    }

    // lock time
    if (tx->buffer_len < offset + 4) return 1;
    offset += read_uint32_le(tx->buffer + offset, &tx->lock_time);
    if (tx->buffer_len < offset) return 1;

    return 0;
}

int btc_tx_get_input(btc_tx_t* tx, int32_t index, btc_tx_input_t* input) {
    int ret = 1;
    int offset = tx->input_index;
    for (int i = 0; i < tx->input_len; i++) {
        // hash
        for (int j = 0; j < 32; j++) {
            input->hash[32 - j - 1] = tx->buffer[offset + j];
        }
        offset += 32;
        offset += read_uint32_le(tx->buffer + offset, &input->index);  // index
        // script
        offset += decode_var_int(tx->buffer + offset, &input->script_len);
        input->script = tx->buffer + offset;
        offset += input->script_len;
        offset += read_uint32_le(tx->buffer + offset, &input->sequence);  // sequence
        if (i == index) {
            ret = 0;
            break;
        }
    }
    return ret;
}

int btc_tx_get_output(btc_tx_t* tx, int32_t index, btc_tx_output_t* output) {
    int ret = 1;
    int offset = tx->output_index;
    for (int i = 0; i < tx->output_len; i++) {
        offset += read_uint64_le(tx->buffer + offset, &output->value);  // index
        // script
        offset += decode_var_int(tx->buffer + offset, &output->script_len);
        output->script = tx->buffer + offset;
        offset += output->script_len;
        if (i == index) {
            ret = 0;
            break;
        }
    }
    return ret;
}

int btc_tx_hash_for_signature(btc_tx_t* tx, uint16_t index, sighash_type_t type, uint8_t * prev_out_script, uint16_t script_len, hash_func_t hasher, void* param) {
    if ((type & 0x1f) != SIGHASH_ALL) {
        return -2;
    }

    uint8_t output[32];
    int offset = 0;
    int ret;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_BEGIN);

    // version
    offset += write_uint32_le(output + offset, tx->version);
    // [output hash]
    HASHER(hasher, param, output, offset, STEP_VERSION);

    if (tx->mask & BTC_TX_MASK_NTIME) {
        // ntime
        for (int i = 0; i < 4; i++) {
            output[offset + i] = tx->buffer[4 + i];
        }
        offset += 4;
        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
    }

    if (tx->has_witnesses) {
        output[offset++] = ADVANCED_TRANSACTION_MARKER;
        output[offset++] = ADVANCED_TRANSACTION_FLAG;
    }

    // inputs
    offset += encode_var_int(output + offset, tx->input_len);

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
    
    for (int i = 0; i < tx->input_len; i++) {
        btc_tx_input_t input;
        btc_tx_get_input(tx, i, &input);

        // hash
        for (int j = 0; j < 32; j++) {
            output[offset + j] = input.hash[32 - j - 1];
        }
        offset += 32;

        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

        // index
        offset += write_uint32_le(output + offset, input.index);

        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

        // script
        if (i == index) {
            offset += encode_var_int(output + offset, script_len);
            for (int j = 0; j < script_len; j++) {
                // [output hash]
                if (offset >= 32) {
                    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
                }

                output[offset++] = prev_out_script[j];
            }
        }
        else {
            output[offset++] = 0;
        }

        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

        // sequence
        offset += write_uint32_le(output + offset, input.sequence);

        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
    }

    // outputs
    offset += encode_var_int(output + offset, tx->output_len);
    for (int i = 0; i < tx->output_len; i++) {
        btc_tx_output_t o;
        btc_tx_get_output(tx, i, &o);

        // value
        offset += write_uint64_le(output + offset, o.value);

        // script
        offset += encode_var_int(output + offset, o.script_len);
        for (int j = 0; j < o.script_len; j++) {

            // [output hash]
            if (offset >= 32) {
                HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
            }

            output[offset++] = o.script[j];
        }

        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
    }

    // witnesses
    if (tx->has_witnesses) {
        for (int i = 0; i < tx->witnesses_len; i++) {

            // [output hash]
            if (offset >= 32) {
                HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
            }

            output[offset++] = tx->buffer[tx->witnesses_index + i];
        }

        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
    }

    // lock time
    offset += write_uint32_le(output + offset, tx->lock_time);

    // sign type
    offset += write_uint32_le(output + offset, type);

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_END);

    return 0;
}

int btc_hash_outputs(btc_tx_t* tx, sighash_type_t type, hash_func_t hasher, void* param) {
    uint8_t output[32];
    int offset = 0;
    int ret = 0;

    // init hasher
    HASHER(hasher, param, output, offset, STEP_BEGIN);

    for (int i = 0; i < tx->output_len; i++) {
        btc_tx_output_t o;
        btc_tx_get_output(tx, i, &o);

        // value
        offset += write_uint64_le(output + offset, o.value);

        // script
        offset += encode_var_int(output + offset, o.script_len);
        for (int j = 0; j < o.script_len; j++) {

            // [output hash]
            if (offset >= 32) {
                HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
            }

            output[offset++] = o.script[j];
        }

        // [output hash]
        HASHER(hasher, param, output, offset, i == tx->output_len - 1 ? STEP_END : STEP_CONTINUOUS);
    }
    return ret;
}

int btc_hash_inputs(btc_tx_t* tx, sighash_type_t type, hash_func_t hasher, void* param) {
    uint8_t output[32];
    int offset = 0;
    int ret = 0;

    // init hasher
    HASHER(hasher, param, output, offset, STEP_BEGIN);

    for (int i = 0; i < tx->input_len; i++) {
        btc_tx_input_t input;
        btc_tx_get_input(tx, i, &input);

        // hash
        for (int j = 0; j < 32; j++) {
            output[offset + j] = input.hash[32 - j - 1];
        }
        offset += 32;

        // [output hash]
        HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

        // index
        offset += write_uint32_le(output + offset, input.index);

        // [output hash]
        HASHER(hasher, param, output, offset, i == tx->input_len - 1 ? STEP_END : STEP_CONTINUOUS);
    }
    return ret;
}

int btc_hash_sequence(btc_tx_t* tx, sighash_type_t type, hash_func_t hasher, void* param) {
    uint8_t output[32];
    int offset = 0;
    int ret = 0;

    // init hasher
    HASHER(hasher, param, output, offset, STEP_BEGIN);

    for (int i = 0; i < tx->input_len; i++) {
        btc_tx_input_t input;
        btc_tx_get_input(tx, i, &input);

        // sequence
        offset += write_uint32_le(output + offset, input.sequence);

        // [output hash]
        HASHER(hasher, param, output, offset, i == tx->input_len - 1 ? STEP_END : STEP_CONTINUOUS);
    }
    return ret;
}

int btc_tx_hash_for_witness_v0(btc_tx_t* tx, uint16_t index, sighash_type_t type, uint8_t * prev_out_script, uint16_t script_len, uint64_t value, uint8_t * hash_inputs, uint8_t * hash_sequence, uint8_t * hash_outputs, hash_func_t hasher, void* param) {
    if ((type & SIGHASH_ALL) == 0) {
        return -2;
    }

    uint8_t output[32];
    int offset = 0;
    int ret;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_BEGIN);

    // version
    offset += write_uint32_le(output + offset, tx->version);

    if (tx->mask & BTC_TX_MASK_NTIME) {
        // ntime
        for (int i = 0; i < 4; i++) {
            output[offset + i] = tx->buffer[4 + i];
        }
        offset += 4;
    }

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // inputs
    for (int j = 0; j < 32; j++) {
        output[offset + j] = hash_inputs[j];
    }
    offset += 32;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // sequence
    for (int j = 0; j < 32; j++) {
        output[offset + j] = hash_sequence[j];
    }
    offset += 32;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // input hash
    btc_tx_input_t input;
    btc_tx_get_input(tx, index, &input);
    for (int j = 0; j < 32; j++) {
        output[offset + j] = input.hash[32 - j - 1];
    }
    offset += 32;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // input index
    offset += write_uint32_le(output + offset, input.index);

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // input script
    offset += encode_var_int(output + offset, script_len);
    for (int j = 0; j < script_len; j++) {
        // [output hash]
        if (offset >= 32) {
            HASHER(hasher, param, output, offset, STEP_CONTINUOUS);
        }

        output[offset++] = prev_out_script[j];
    }

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // input value
    offset += write_uint64_le(output + offset, value);

    // input sequence
    offset += write_uint32_le(output + offset, input.sequence);

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // outputs
    for (int j = 0; j < 32; j++) {
        output[offset + j] = hash_outputs[j];
    }
    offset += 32;

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_CONTINUOUS);

    // lock time
    offset += write_uint32_le(output + offset, tx->lock_time);

    // sign type
    offset += write_uint32_le(output + offset, type);

    // [output hash]
    HASHER(hasher, param, output, offset, STEP_END);

    return 0;
}
