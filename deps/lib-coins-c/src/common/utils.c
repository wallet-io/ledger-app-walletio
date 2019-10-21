#include "utils.h"

int32_t read_uint16_le(const uint8_t * buffer, uint16_t * out) {
    ((uint8_t*)out)[0] = buffer[0];
    ((uint8_t*)out)[1] = buffer[1];
    return 2;
}

int32_t read_uint32_le(const uint8_t * buffer, uint32_t * out) {
    ((uint8_t*)out)[0] = buffer[0];
    ((uint8_t*)out)[1] = buffer[1];
    ((uint8_t*)out)[2] = buffer[2];
    ((uint8_t*)out)[3] = buffer[3];
    return 4;
}

int32_t read_uint64_le(const uint8_t * buffer, uint64_t * out) {
    ((uint8_t*)out)[0] = buffer[0];
    ((uint8_t*)out)[1] = buffer[1];
    ((uint8_t*)out)[2] = buffer[2];
    ((uint8_t*)out)[3] = buffer[3];
    ((uint8_t*)out)[4] = buffer[4];
    ((uint8_t*)out)[5] = buffer[5];
    ((uint8_t*)out)[6] = buffer[6];
    ((uint8_t*)out)[7] = buffer[7];
    return 8;
}

int32_t read_uint16_be(const uint8_t * buffer, uint16_t * out) {
    ((uint8_t*)out)[0] = buffer[1];
    ((uint8_t*)out)[1] = buffer[2];
    return 2;
}

int32_t read_uint32_be(const uint8_t * buffer, uint32_t * out) {
    ((uint8_t*)out)[0] = buffer[3];
    ((uint8_t*)out)[1] = buffer[2];
    ((uint8_t*)out)[2] = buffer[1];
    ((uint8_t*)out)[3] = buffer[0];
    return 4;
}

int32_t read_uint64_be(const uint8_t * buffer, uint64_t * out) {
    ((uint8_t*)out)[0] = buffer[7];
    ((uint8_t*)out)[1] = buffer[6];
    ((uint8_t*)out)[2] = buffer[5];
    ((uint8_t*)out)[3] = buffer[4];
    ((uint8_t*)out)[4] = buffer[3];
    ((uint8_t*)out)[5] = buffer[2];
    ((uint8_t*)out)[6] = buffer[1];
    ((uint8_t*)out)[7] = buffer[0];
    return 8;
}

int32_t write_uint16_le(uint8_t * buffer, uint16_t value) {
    buffer[0] = ((uint8_t*)&value)[0];
    buffer[1] = ((uint8_t*)&value)[1];
    return 2;
}

int32_t write_uint32_le(uint8_t * buffer, uint32_t value) {
    buffer[0] = ((uint8_t*)&value)[0];
    buffer[1] = ((uint8_t*)&value)[1];
    buffer[2] = ((uint8_t*)&value)[2];
    buffer[3] = ((uint8_t*)&value)[3];
    return 4;
}

int32_t write_uint64_le(uint8_t * buffer, uint64_t value) {
    buffer[0] = ((uint8_t*)&value)[0];
    buffer[1] = ((uint8_t*)&value)[1];
    buffer[2] = ((uint8_t*)&value)[2];
    buffer[3] = ((uint8_t*)&value)[3];
    buffer[4] = ((uint8_t*)&value)[4];
    buffer[5] = ((uint8_t*)&value)[5];
    buffer[6] = ((uint8_t*)&value)[6];
    buffer[7] = ((uint8_t*)&value)[7];
    return 8;
}

int32_t write_uint16_be(uint8_t * buffer, uint16_t value) {
    buffer[0] = ((uint8_t*)&value)[1];
    buffer[1] = ((uint8_t*)&value)[0];
    return 2;
}

int32_t write_uint32_be(uint8_t * buffer, uint32_t value) {
    buffer[0] = ((uint8_t*)&value)[3];
    buffer[1] = ((uint8_t*)&value)[2];
    buffer[2] = ((uint8_t*)&value)[1];
    buffer[3] = ((uint8_t*)&value)[0];
    return 4;
}

int32_t write_uint64_be(uint8_t * buffer, uint64_t value) {
    buffer[0] = ((uint8_t*)&value)[7];
    buffer[1] = ((uint8_t*)&value)[6];
    buffer[2] = ((uint8_t*)&value)[5];
    buffer[3] = ((uint8_t*)&value)[4];
    buffer[4] = ((uint8_t*)&value)[3];
    buffer[5] = ((uint8_t*)&value)[2];
    buffer[6] = ((uint8_t*)&value)[1];
    buffer[7] = ((uint8_t*)&value)[0];
    return 8;
}

int32_t decode_var_int(const uint8_t * buffer, int64_t * out) {
    int32_t offset = 0;
    uint8_t value;
    value = buffer[offset];
    
    if (value < 0xfd) { // 8 bit
        *out = value;
        offset = 1;
    }
    else if (value == 0xfd) { // 16 bit
        uint16_t v;
        read_uint16_le(buffer + offset + 1, &v);
        *out = v;
        offset = 3;
    }
    else if (value == 0xfe) { // 32 bit
        uint32_t v;
        read_uint32_le(buffer + offset + 1, &v);
        *out = v;
        offset = 5;
    }
    else { // 64 bit
        uint64_t v;
        read_uint64_le(buffer + offset + 1, &v);
        *out = v;
        offset = 9;
    }
    
    return offset;
}

int32_t encode_var_int(uint8_t * buffer, int64_t value) {
    int32_t offset = 0;
    
    if (value < 0xfd) { // 8 bit
        buffer[0] = ((uint8_t*)&value)[0];
        offset = 1;
    }
    else if (value <= 0xffff) { // 16 bit
        buffer[0] = 0xfd;
        write_uint16_le(buffer + 1, (uint16_t)value);
        offset = 3;
    }
    else if (value <= 0xffffffff) { // 32 bit
        buffer[0] = 0xfe;
        write_uint32_le(buffer + 1, (uint32_t)value);
        offset = 5;
    }
    else { // 64 bit
        buffer[0] = 0xff;
        write_uint64_le(buffer + 1, value);
        offset = 9;
    }
    
    return offset;
}

int buffer_to_hex(const uint8_t * buffer, size_t buffer_len, uint8_t * out, size_t out_len) {
    uint8_t map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    int i;
    for(i = 0; i < buffer_len && i < out_len / 2; i++) {
        uint8_t b = (buffer[i] >> 4) & 0x0f;
        uint8_t l = buffer[i] & 0x0f;
        out[i * 2] = map[b];
        out[i * 2 + 1] = map[l];
    }

    return i * 2;
}

static uint8_t convert_hex_to_digital(uint8_t c) {
    if (c >= '0' && c <= '9') {
        c -= '0';
    }
    else if (c >= 'A' && c <= 'F') {
        c = c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f') {
        c = c - 'a' + 10;
    }
    else {
        return 0xff;
    }

    return c;
}

int hex_to_buffer(const uint8_t * hex, size_t hex_len, uint8_t * out, size_t out_len) {
    int padding = hex_len % 2;
    uint8_t last_digital = 0;
    int i;
    for(i = 0; i < hex_len && i < out_len * 2; i++) {
        uint8_t d = convert_hex_to_digital(hex[i]);
        if (d > 0x0f) {
            break;
        }

        if ((i + padding) % 2) {
            out[(i + padding) / 2] = ((last_digital << 4) & 0xf0) | (d & 0x0f);
        }
        else {
            last_digital = d;
        }
    }

    return (i + padding) / 2;
}
uint8_t ch_to_num(uint8_t ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 55;
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 87;
  }
}
uint32_t hex_to_buf(uint8_t* in, uint32_t in_len, uint8_t* out, uint32_t out_capacity) {
  /*
   * traversal in
   * converts hex character to decimal digit
   * shift
   * copy to out
   * return out len
   */
  uint32_t out_index = 0;
  if (in == NULL || out == NULL || in_len < 1) return out_index;
  for (uint32_t index = 0; index < in_len;) {
    if (index + 2 > in_len) {
      out[out_index++] = ch_to_num(in[index++]);
    } else {
      out[out_index++] = (ch_to_num(in[index++]) << 4) | ch_to_num(in[index++]);
    }
  }
  return out_index - 1;
}

