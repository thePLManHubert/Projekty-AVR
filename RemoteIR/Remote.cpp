#include "Remote.h"
#include <avr/interrupt.h>

Remote::Remote( uint8_t frequency ) // kHz
		: mfrequency( frequency ) {
	init();
}

void Remote::send( uint32_t data ) {
	transmitOn = true;
	sendHeader();
	sendPause();
	for (uint8_t i = 0; i < 32; i++) {
		if (data & 0x80000000UL)
			sendOne();
		else sendZero();
		data <<= 1;
	}
	transmitOn = false;
}

void Remote::init() {
	TRANSMIT_PORT_DDR |= _BV( TRANSMIT_PIN );
	OCR2 = F_CPU / 2000 / mfrequency; // dla czêstotliwoœci odbiornika frequency [kHz]
	OCR1A = 24;	// do okreœlania d³ugoœci impulsu (timer <-> timer+1) == 100us
	//preskaler 1, taktowanie 16 MHz, tryb CTC
	TCCR2 |= _BV(CS20) | _BV( WGM21 );
	//preskaler 64, taktowanie 250000 Hz, tryb CTC
	TCCR1B = _BV(CS11) | _BV( CS10 ) | _BV( WGM12 );
	// przerwania zegarowe w³¹czone
	TIMSK = _BV( OCIE2 ) | _BV( OCIE1A );
	// przerwania globalne w³¹czone
	sei();
}

void Remote::sendOne() {
	TCNT1 = 0;
	timer = 0;
	while (timer < 16)
		S_LOW;
	TCNT1 = 0;
	timer = 0;
	while (timer < 8) {
		if (state)
			S_HIGH;
		else S_LOW;
	}

}

void Remote::sendZero() {
	TCNT1 = 0;
	timer = 0;
	while (timer < 5)
		S_LOW;
	TCNT1 = 0;
	timer = 0;
	while (timer < 8) {
		if (state)
			S_HIGH;
		else S_LOW;
	}

}

void Remote::sendHeader() {
	TCNT1 = 0;
	timer = 0;
	while (timer < 90) {
		if (state)
			S_HIGH;
		else S_LOW;
	}
}

void Remote::sendPause() {
	TCNT1 = 0;
	timer = 0;
	while (timer < 43)
		S_LOW;
	TCNT1 = 0;
	timer = 0;
	while (timer < 8) {
		if (state)
			S_HIGH;
		else S_LOW;
	}
}

ISR( TIMER2_COMP_vect ) {
	if (transmitOn)
		state = !state;
	else {
		state = false;
		S_LOW;
	}
}

ISR( TIMER1_COMPA_vect ) {
	++timer;
}
