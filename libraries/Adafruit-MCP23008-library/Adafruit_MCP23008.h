/*************************************************** 
  This is a library for the MCP23008 i2c port expander

  These displays use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#ifndef _ADAFRUIT_MCP23008_H
#define _ADAFRUIT_MCP23008_H



// Determine board type
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  // Regular Arduino
  #warning Using Hardware I2C
  #ifndef HARD_I2C
    #define HARD_I2C
  #endif
#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
  // Arduino Mega
  #warning Using Software I2C
  #ifndef SOFT_I2C
    #define SOFT_I2C
  #endif
#else
  #warning untested board - please check your I2C ports
#endif

#ifdef HARD_I2C

#include <Wire.h>

class Adafruit_MCP23008 {
public:
  void begin(uint8_t addr);
  void begin(void);

  void pinMode(uint8_t p, uint8_t d);
  void digitalWrite(uint8_t p, uint8_t d);
  void pullUp(uint8_t p, uint8_t d);
  uint8_t digitalRead(uint8_t p);
  uint8_t readGPIO(void);
  void writeGPIO(uint8_t);

 private:
  uint8_t i2caddr;
  uint8_t read8(uint8_t addr);
  void write8(uint8_t addr, uint8_t data);
};

#define MCP23008_ADDRESS 0x20

// registers
#define MCP23008_IODIR 0x00
#define MCP23008_IPOL 0x01
#define MCP23008_GPINTEN 0x02
#define MCP23008_DEFVAL 0x03
#define MCP23008_INTCON 0x04
#define MCP23008_IOCON 0x05
#define MCP23008_GPPU 0x06
#define MCP23008_INTF 0x07
#define MCP23008_INTCAP 0x08
#define MCP23008_GPIO 0x09
#define MCP23008_OLAT 0x0A

#endif

#endif // HARD_I2C
