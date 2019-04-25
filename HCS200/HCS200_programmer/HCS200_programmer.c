#include <avr/io.h>
#include <util/delay.h>
#include "HCS200_programmer.h"


package pack( uint16_t *buffer, data *d )
{

	buffer[0]  = d->encryption_key >>  0;	//LSB
	buffer[1]  = d->encryption_key >> 16;
	buffer[2]  = d->encryption_key >> 32;
	buffer[3]  = d->encryption_key >> 48;	//MSB

	buffer[4]  = d->sync;
	buffer[5]  = 0x0000;

	buffer[6]  = d->serial_number >>  0;
	buffer[7]  = d->serial_number >> 16;

	buffer[8]  = d->seed >>  0;
	buffer[9]  = d->seed >> 16;
	buffer[10] = 0x0000;

	buffer[11] = d->config;

	return (package)buffer;

}


/*
data unpack ( uint16_t *buffer, package p ){ // not tested

	data *d = (data*)buffer;
	uint16_t * _word = p;

	d->encryption_key = *(_word + 3);
	d->encryption_key <<= 16 + *(_word + 2);
	d->encryption_key <<= 16 + *(_word + 1);
	d->encryption_key <<= 16 + *_word;

	d->sync = *(_word + 4);

	d->serial_number = *(_word + 7);
	d->serial_number <<= 16 + *(_word + 6);

	d->seed = *(_word + 9);
	d->seed <<= 16 + *(_word + 8);

	d->config = *(_word + 11);

	return *d;

}
*/


void load( package p ){

	PROGRAM_PORT_DIR |= _BV(CLK_PIN) | _BV(DATA_PIN);

	// start
	PROGRAM_PORT |= _BV(CLK_PIN);
	_delay_ms(4);
	PROGRAM_PORT |= _BV(DATA_PIN);
	_delay_ms(4);
	PROGRAM_PORT ^= _BV(DATA_PIN);
	_delay_us(50);
	PROGRAM_PORT ^= _BV(CLK_PIN);
	_delay_ms(3.5);
	// end of start

	// sending
	for(uint16_t *_word = p; _word < (p + 12); _word++){

		for(uint8_t _bit = 0; _bit < 16; _bit++){

			PROGRAM_PORT |= _BV(CLK_PIN);
			PROGRAM_PORT = (*_word & _BV(_bit)) ? PROGRAM_PORT | _BV(DATA_PIN) : PROGRAM_PORT & ~_BV(DATA_PIN);
			_delay_us(26);
			PROGRAM_PORT ^= _BV(CLK_PIN);
			_delay_us(30);

		}

		// holding
		PROGRAM_PORT &= ~(_BV(CLK_PIN) | _BV(DATA_PIN));
		_delay_ms(35);
		// end of holding

	}
	// end of sending

}



bool verify( package p ){

	// start
	bool check = 1;
	PROGRAM_PORT_DIR &= ~_BV(DATA_PIN);
	_delay_us(20);

	for(uint16_t *_word = p; _word < (p + 12); _word++){

		for(uint8_t _bit = 0; _bit < 16; _bit++){

			PROGRAM_PORT |= _BV(CLK_PIN);
			_delay_us(30);
			PROGRAM_PORT &= ~_BV(CLK_PIN);
			_delay_us(5);
			if(!(PROGRAM_PORT_READ & _BV(DATA_PIN)) == (*_word & _BV(_bit)))
				check = 0;
			_delay_us(21);

		}
	}
	// end of start

	return check;

}















