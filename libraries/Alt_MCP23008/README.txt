This library, Alt_MCP23008, has been forked from the Adafruit MCP23008 library,
to provide better compatibility with the PCF8574 I2C expander.

A key difference between the two chips is that the PCF8574 does not require that
you re-address the GPIO port (Address 0x09) before sending subsequent data bytes.

Code that supports both chips using the same code tends to still send out the 0x09
before each data byte, meaning that 0x09 is output on the port briefly before
the actual data that was to be sent.


Original repository:
https://github.com/adafruit/Adafruit-MCP23008-library


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