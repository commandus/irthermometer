#include <stdio.h>
#include "crc8.h"

int main(int argc, char **argv) {
    // read 5A-> B4 ,read RAM=07, result 3A D2 => B407B5D23A, PEC 30
    uint8_t r[5] = { 0xb4, 0x07, 0xb5, 0xd2, 0x3a};
    uint8_t pec = crc8(r, 5);
    printf("read 0x3ad2 b407b5d23a pec: %x\n", pec);

    // write SA -> B4 write EEPROM=0x02, data=0xC807 => b42207c8 PEC=0x48
    uint8_t w[4] = { 0xb4, 0x22, 0x07, 0xc8};
    pec = crc8(w, 4);
    printf("write 0xc807 b42207c8 pec: %x\n", pec);

}