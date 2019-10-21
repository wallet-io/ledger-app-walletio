#include "common_utils.h"

bool platform_is_big_endian() {
    int num = 65538;        // 00000000 00000001 00000000 00000010
    int len = sizeof(num);  // 4
    char *ptr = (char *)&num;  // 第1个字节的内容是1则是小端字节序，为0则是大端字节序
    if (*ptr == 2 && *(ptr + 1) == 0 && *(ptr + 2) == 1 && *(ptr + 3) == 0) {
        return false;
    } else {
        return true;
    }
}

uint8_t base_convert(const uint8_t *buffer, const uint8_t buffer_len, uint8_t *out, const uint8_t out_capcity, const uint8_t old_base, const uint8_t new_base) {
  uint8_t buffer1[buffer_len];
  uint8_t out_reverse[out_capcity];
  uint8_t tmp_result[buffer_len];
  uint8_t out_len = 0;
  char div_index, quotient_digits;
  memcpy(buffer1, buffer, buffer_len);
  for (char m = buffer_len; m > 0;) { // m=0 时没有除数, 退出
    int remainder = 0;                // 保存每一次除的结果的余数
    memset(tmp_result, 0, buffer_len);
    for (div_index = 0; div_index < m; div_index++) {
      remainder = remainder * old_base + buffer1[div_index];
      tmp_result[div_index] = remainder / new_base;
      remainder %= new_base;
    }

    quotient_digits = div_index; // 储存本轮除运算结束后的商的位数，为下轮准备
    out_reverse[out_len++] = remainder; // 最后一次取得的余数作为本轮的取余结果储存并等待输出

    div_index = 0;
    while (div_index < quotient_digits && 0 == tmp_result[div_index]) { // 去除商中的前导 0
      div_index++;
    }

    memset(buffer1, 0, buffer_len); // 清除本轮的除数
    m = 0;
    for (div_index; div_index < quotient_digits; div_index++) {
      buffer1[m++] = tmp_result[div_index]; // 将这一轮的商作为下一轮的除数，得到下一轮除数,
                                            // 保存到str中
    }
  }
  int index = out_len - 1, out_index = 0;
  while (index + 1) {
      out[out_index++] = out_reverse[index];
      index--;
  }
  return out_len;
}

uint8_t uint8_extension(const uint8_t *src, const uint8_t src_len, uint8_t *out, uint8_t out_capcity) {
    if (out_capcity < src_len * 2) return 1;
    uint8_t buffer_index = 0;

    uint8_t index = 0;
    while(src[index] == 0x00) index ++;
    for (; index < src_len; index++) {
        out[buffer_index++] = src[index] >> 4;
        out[buffer_index++] = src[index] & 0x0f;
    }
    return buffer_index;
}

bool num2str(uint8_t *src, uint8_t src_len) {
    for (uint8_t i = 0; i < src_len; i++) {
        src[i] += 48;
    }
    src[src_len] = '\0';
}
/**
 * style
 * 0st bit 0 star  1 dot
 * 1st bit 0 cut end  1 cut middle
 */
bool str_cut(const uint8_t *src, uint8_t src_len, uint8_t *out, uint8_t out_len, uint8_t style) {
  if (src_len < 10 && out_len > src_len){
    strncpy(out, src, src_len);
    return true;
  }

  if (style & 0x02) { // handle cut middle
    uint8_t left_len = (out_len - 3) / 2;
    uint8_t right_len = out_len - left_len - 3;
    if (left_len > src_len || right_len > src_len) {
      return false;
    }

    strncpy(out, src, left_len);
    strncpy(out + left_len, style & 0x01 ? "..." : "***", 3);
    strncpy(out + left_len + 3, src + (src_len - 1 - right_len), right_len);
  } else { // handle cut end
    if (out_len - 3 > src_len) {
      return false;
    }
    strncpy(out, src, out_len - 3);
    strncpy(out + out_len - 3, style & 0x01 ? "..." : "***", 3);
  }
  return true;
}

uint8_t remove_lead_zero(uint8_t *src, uint8_t src_len, uint8_t **out) {
  uint8_t index = 0;
  while (src[index] == 0x00 && index < src_len) index++;
  *out = src + index;
  return src_len - index;
}