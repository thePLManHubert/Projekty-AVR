/*
 * lcd.h
 *
 *  Created on: 27 lut 2018
 *      Author: Hubert
 */

#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>

#define LCD_RS_PIN  PORTB2
#define LCD_E_PIN   PORTB1
#define LCD_RS_PORT PORTB
#define LCD_E_PORT  PORTB
#define LCD_RS_DDR  DDRB
#define LCD_E_DDR   DDRB

#define LCD_RS_LOW  LCD_RS_PORT &= ~_BV(LCD_RS_PIN)
#define LCD_E_LOW   LCD_E_PORT  &= ~_BV(LCD_E_PIN)
#define LCD_RS_HIGH LCD_RS_PORT |= _BV(LCD_RS_PIN)
#define LCD_E_HIGH  LCD_RS_PORT |= _BV(LCD_E_PIN)

#define DATA_PORT_DDR  DDRA
#define DATA_PORT      PORTA
#define DATA_PIN_3     PORTA3 // MSB
#define DATA_PIN_2     PORTA2
#define DATA_PIN_1     PORTA1
#define DATA_PIN_0     PORTA0 // LSB

#define LCD_COMMAND_CLEAR 0x01
#define LCD_COMMAND_RETURN_HOME 0x02
#define LCD_COMMAND_ENTRY_MODE_SET 0x04
#define LCD_COMMAND_ON_OFF 0x08
#define LCD_COMMAND_SHIFT 0x10
#define LCD_COMMAND_FUNCTION_SET 0x20
#define LCD_COMMAND_SET_CGRAM_ADDRESS 0x40
#define LCD_COMMAND_SET_DDRAM_ADDRESS 0x80

#define LCD_PARAM_ENTRY_MODE_SET_SHIFT 0x01
#define LCD_PARAM_ENTRY_MODE_SET_INCREMENT 0x02
#define LCD_PARAM_ON_OFF_BLINK 0x01
#define LCD_PARAM_ON_OFF_CURSOR 0x02
#define LCD_PARAM_ON_OFF_DISPLAY 0x04
#define LCD_PARAM_SHIFT_RIGHT 0x04
#define LCD_PARAM_SHIFT_DISPLAY 0x08
#define LCD_PARAM_FUNCTION_SET_5X10 0x04
#define LCD_PARAM_FUNCTION_SET_2LINES 0x08
#define LCD_PARAM_FUNCTION_SET_8BIT 0x10

class Lcd{
public:
	Lcd();
	void print( int number );
	void print( const char *str );
	void goToXY( uint8_t x, uint8_t y );
	void clear();

private:
	void write( uint8_t data, uint8_t rs, uint8_t checkBusyFlag);
	void writeCommand( uint8_t command );
	void writeData( uint8_t data );
	void init();

};


#endif /* LCD_H_ */
