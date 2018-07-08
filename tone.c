#include<avr/pgmspace.h>
const char tone_data[3][41] PROGMEM = {
  {
  //Acordion
  0x35,0x20,0x08,0x02,0x00,0x00,0x01,0x15,0x00,0x63,0x11,0x01,0x07,0x02,0x00,0x0A,0x02,0x02,0x01,0x71,
  0x00,0x00,0x06,0x0F,0x00,0x00,0x01,0x12,0x00,0x21,0x11,0x02,0x07,0x0F,0x00,0x0A,0x00,0x07,0x01,0x32,0x00,
  },
  {
  //chrch org
  0x2F,0x02,0x09,0x0F,0x00,0x05,0x00,0x13,0x00,0x03,0x00,0x00,0x0B,0x07,0x00,0x02,0x02,0x1D,0x00,0x07,
  0x00,0x02,0x08,0x0F,0x00,0x05,0x00,0x04,0x00,0x01,0x00,0x02,0x08,0x07,0x00,0x05,0x00,0x04,0x00,0x00,0x05,
  },
  {
  //violin
  0x35,0x26,0x06,0x00,0x00,0x03,0x00,0x12,0x10,0x01,0x0C,0x00,0x06,0x04,0x00,0x07,0x02,0x03,0x01,0x51,
  0x00,0x25,0x0E,0x05,0x07,0x0A,0x00,0x06,0x00,0x04,0x06,0x02,0x06,0x07,0x07,0x07,0x0F,0x03,0x03,0x11,0x00,
  }
};

