/**
 * mlx90614 https://www.melexis.com/-/media/files/documents/datasheets/mlx90614-datasheet-melexis.pdf
 *  Pins: 
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
 */
#include <DigiCDC.h>
#include <TinyWireM.h>

int TINY_SDA =     0; // ATtiny SDA pin 5
int TINY_SCL =     2; // ATtiny SCL pin 7
int TINY_LED =     1; // ATtiny LED

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

/**
 * Kelvin = Celcius + 273.15
 * Return temperature K degrees * 100
 */
uint16_t readK100(uint8_t registerAddress) {
  TinyWireM.beginTransmission(MLX90614_I2CADDR); // start transmission to device 
  TinyWireM.write(registerAddress); // sends register address to read from
  TinyWireM.endTransmission(false); // end transmission
  // send data n-bytes read
  TinyWireM.requestFrom(MLX90614_I2CADDR, (uint8_t) 3);
  // receive DATA
  uint16_t ret = TinyWireM.read(); 
  // receive DATA
  ret |= TinyWireM.read() << 8;
  uint8_t pec = TinyWireM.read();
  // *2
  return ret << 1;
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

int c = 0;
uint16_t data[2] = {-1, -1};

// the loop routine runs over and over again forever:
void loop() {
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
  // keep usb alive, can also use SerialUSB.refresh();
  SerialUSB.delay(100);
  // Object temperature
  data[0] = readK100(MLX90614_TOBJ1);
  // Ambient temperature
  data[1] = readK100(MLX90614_TA);
}
