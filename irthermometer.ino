/**
 * 
 * Read temperature from IR thermometer in Kelvin degrees * 100
 * 
 * andrey.ivanov@ikfia.ysn.ru
 * 
 * Connect to COM port (Digispark bootloader runs code after 5s delay)
 * 
 * Send '0' character to read temperature from IR sensor
 * Send '1' character to read temperature from the chip (ambient temperature)
 * Send '2' character to read program loops(for debug)
 * Send '3' character to read last error
 * 
 * Digispark AtTiny85 blinks when character has been received and respond
 * with decimal number e.g.
 * 27315 means 0C
 * 
 * mlx90614 https://www.melexis.com/-/media/files/documents/datasheets/mlx90614-datasheet-melexis.pdf
 * 
 * Pins: 
 *  
 *    0: SDA
 *    2: SCL
 *
 * 1. File/Preferences Add entry in Additional Boards Manager URLs: http://digistump.com/package_digistump_index.json
 * 2. Tools/Board/Boards Manager install Digispark
 * 3. Tools/Board Select Digispark (Default 16.5mhz)
 * 4. Optional Sketch/Manage Libraries, install Adafruit_MiniMLX90614
 *    See https://www.arduinolibraries.info/libraries/adafruit-mini-mlx90614 https://github.com/adafruit/Adafruit_MiniMLX90614
 *    Example project https://www.instructables.com/id/IR-Thermometer/
 *    
 *    Errors returned on '3':
 *    
 *    1 The slave did not acknowledge the address
 *    2 The slave did not acknowledge all data
 *    3 Generated Start Condition not detected on bus
 *    4 Generated Stop Condition not detected on bus
 *    5 Unexpected Data Collision (arbitration)
 *    6 Unexpected Stop Condition
 *    7 Unexpected Start Condition
 *    8 Transmission buffer is empty
 *    9 Transmission buffer is outside SRAM space
 *    10 Error during external memory read
 *    
 *    20 CRC error
 */
#include <DigiCDC.h>
#include <TinyWireM.h>

// ATtiny LED
#define TINY_LED 1

#define MLX90614_I2CADDR 0x5A
// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x0E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F

#define ERR_CODE_BAD_CRC 20

/**
 * @see https://crccalc.com/
 */

uint8_t crc8(
  const void* data,
  int size
) {
  const uint8_t *a = (const uint8_t *) data;
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

/*
static uint8_t crc8Table[] = {
  0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
  0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
  0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
  0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
  0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
  0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
  0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
  0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
  0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
  0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
  0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
  0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
  0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
  0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
  0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
  0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
  0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
  0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
  0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
  0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
  0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
  0xfa, 0xfd, 0xf4, 0xf3
};

uint8_t crc8(
    const void* data,
    int size
)
{
  const uint8_t *a = (const uint8_t *) data;
  uint8_t crc = 0;
  int j;
  for (j = 0; j < size; j++, a++) {
    crc = crc8Table[crc ^ *a];
  }
  return crc;
}
*/

/**
 * crcbuffer[3] MSB crcbuffer[4] LSB
 */
static uint8_t crcbuffer[5] = { 0xb4, 0x07, 0xb5, 0x00, 0x00 };

int c = 0;
uint16_t data[4] = {-1, -1, 0, 0 };

/**
 * Kelvin = Celcius + 273.15
 * @param retval temperature K degrees * 100
 * @return 0- OK, or error code
 */
uint8_t readK100(
  uint16_t &retval,
  uint8_t registerAddress
) {
  TinyWireM.beginTransmission(MLX90614_I2CADDR); // start transmission to device 
  TinyWireM.write(registerAddress); // sends register address to read from
  TinyWireM.endTransmission(false); // end transmission
  // send data n-bytes read
  uint8_t r = TinyWireM.requestFrom(MLX90614_I2CADDR, (uint8_t) 3);
  if (r) {
    // error
    return r;
  }
  crcbuffer[1] = registerAddress;
  // receive DATA
  crcbuffer[3] = TinyWireM.read();     // LSB
  crcbuffer[4] = TinyWireM.read();     // MSB
  uint8_t pec = TinyWireM.read();
  // check crc-8
  uint8_t crc = crc8(crcbuffer, 5);
  if (crc == pec) {
    // *2
    retval = (crcbuffer[3] | (crcbuffer[4] << 8)) << 1;
  } else {
    r = ERR_CODE_BAD_CRC;
  }
  return r;
}

void setup() {
  // Initialize serial                
  SerialUSB.begin();
  // wait until serial is initialized
  while (!SerialUSB); 
  // initialize the digital pin as an output.
  pinMode(TINY_LED, OUTPUT);
  // Init MELEXIS sensor
  TinyWireM.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  /**/
  // turns led on and off based on sending 0 or 1 from serial terminal
  if (SerialUSB.available()) {
    // blink LED
    digitalWrite(TINY_LED, c % 2 ? LOW : HIGH);
    c++;
    // '0'- object Celsius degrees * 100, '1'- ambient Celsius degrees * 100
    int ch = SerialUSB.read();
    int idx = ch - '0';
    if (idx >= 0 && idx < (sizeof(data) / sizeof(int))) {
      char s[7];
      char *p = itoa(data[idx], s, 10);
      while (*p) {
        SerialUSB.write(*p);
        p++;
      }
      SerialUSB.write('\r');
      SerialUSB.write('\n');
    }
  }
  /**/
  // keep usb alive, can also use SerialUSB.refresh();
  SerialUSB.delay(10);
  /**/
  // Object temperature
  uint8_t r = readK100(data[0], MLX90614_TOBJ1);
  if (r) {
     // Set data[3] to error code if failed
    data[3] = r;
  }
  // Ambient temperature
  if ((data[2] % 1024) == 0) {
    r = readK100(data[1], MLX90614_TA);  
    if (r) {
      // Set data[3] to error code if failed
      data[3] = r;
    }
  }
  data[2]++;
}
