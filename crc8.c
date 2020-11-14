#include "crc8.h"

/**
 * @see https://crccalc.com/
 */
uint8_t crc8(
    const void* data,
    int size
) {
  const uint8_t *a = data;
  unsigned crc = 0;
  int i, j;
  for (j = size; j; j--, a++) {
    crc ^= (*a << 8);
    for(i = 8; i; i--) {
      if (crc & 0x8000)
        crc ^= (0x1070 << 3);
      crc <<= 1;
    }
  }
  return (uint8_t)(crc >> 8);
}
