/*
 * lcd.cpp
 *
 *  Created on: 27 lut 2018
 *      Author: Hubert Korgul
 */
#include <util/delay.h>
#include <stdlib.h>
#include "lcd.h"


Lcd::Lcd(){
	init();
}


void Lcd::init(){
	LCD_E_DDR |= _BV(LCD_E_PIN);
	LCD_RS_DDR |= _BV(LCD_RS_PIN);
	_delay_ms(15);
	writeCommand(LCD_COMMAND_FUNCTION_SET | LCD_PARAM_FUNCTION_SET_8BIT);
	_delay_ms(4.1);
	writeCommand(LCD_COMMAND_FUNCTION_SET | LCD_PARAM_FUNCTION_SET_8BIT);
	_delay_ms(0.1);
	writeCommand(LCD_COMMAND_FUNCTION_SET | LCD_PARAM_FUNCTION_SET_8BIT);
	_delay_ms(1);
	writeCommand(LCD_COMMAND_FUNCTION_SET);
	_delay_ms(1);
	writeCommand(LCD_COMMAND_FUNCTION_SET | LCD_PARAM_FUNCTION_SET_2LINES);

	writeCommand(LCD_COMMAND_ON_OFF);
	writeCommand(LCD_COMMAND_ON_OFF | LCD_PARAM_ON_OFF_DISPLAY);
	writeCommand(LCD_COMMAND_ENTRY_MODE_SET | LCD_PARAM_ENTRY_MODE_SET_INCREMENT);
	writeCommand(LCD_COMMAND_CLEAR);
	writeCommand( LCD_COMMAND_SET_DDRAM_ADDRESS | 0 );
}


void Lcd::write( uint8_t data, uint8_t rs, uint8_t checkBusyFlag){ // 4bit
	if( checkBusyFlag )
			_delay_ms(1);

	if( rs ) LCD_RS_HIGH; else LCD_RS_LOW;

	DATA_PORT_DDR |= _BV(DATA_PIN_3) | _BV(DATA_PIN_2) | _BV(DATA_PIN_1) | _BV(DATA_PIN_0);

	DATA_PORT = ( data & 0b1000 ) ? DATA_PORT | _BV(DATA_PIN_3) : DATA_PORT & ~_BV(DATA_PIN_3);
	DATA_PORT = ( data & 0b100  ) ? DATA_PORT | _BV(DATA_PIN_2) : DATA_PORT & ~_BV(DATA_PIN_2);
	DATA_PORT = ( data & 0b10   ) ? DATA_PORT | _BV(DATA_PIN_1) : DATA_PORT & ~_BV(DATA_PIN_1);
	DATA_PORT = ( data & 0b1    ) ? DATA_PORT | _BV(DATA_PIN_0) : DATA_PORT & ~_BV(DATA_PIN_0);

	LCD_E_HIGH;
	_delay_us(1);
	LCD_E_LOW;
	_delay_us(1);
}


void Lcd::writeCommand( uint8_t command){
	write( command >> 4, 0, 1 );
	write( command, 0, 0 );
}


void Lcd::writeData( uint8_t data){
	write( data >> 4, 1, 1 );
	write( data, 1, 0 );
}


void Lcd::print( const char *str ){
	while( *str != 0 ){
		writeData( *str++ );
	}
}


void Lcd::print( int number ){
	char buffer[6];
	print(itoa(number, buffer, 10));
}


void Lcd::goToXY( uint8_t x, uint8_t y ){
	writeCommand( LCD_COMMAND_SET_DDRAM_ADDRESS | (x + y * 0x40) );
}


void Lcd::clear(){
	writeCommand(LCD_COMMAND_CLEAR);
	writeCommand( LCD_COMMAND_SET_DDRAM_ADDRESS | 0 );
}
