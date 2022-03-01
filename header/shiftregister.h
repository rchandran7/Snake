#ifndef SHIFTREGISTER_H 
#define SHIFTREGISTER_H

#include <avr/io.h>

void row_data(unsigned char value) {
  PORTC |= 0x02;
  for(int i = 0; i < 8; ++i) {
    PORTC |= 0x10;
    PORTC &= 0xF0;
    PORTC |= ((value >> i) & 0x01);
    PORTC |= 0x08;
  }
  PORTC |= 0x04;
  PORTC &= 0xC0;
}

void column_data(unsigned char value) {
  PORTD |= 0x02;
  for(int i = 0; i < 8; ++i) {
    PORTD |= 0x10;
    PORTD &= 0xF0;
    PORTD |= ((value >> i) & 0x01);
    PORTD |= 0x08;
  }
  PORTD |= 0x04;
  PORTD &= 0xC0;
}
#endif
