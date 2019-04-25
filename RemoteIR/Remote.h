#ifndef REMOTE_H
#define REMOTE_H

#include <avr/io.h>

#define TRANSMIT_PORT_DDR DDRB
#define TRANSMIT_PORT PORTB
#define TRANSMIT_PIN  PORTB0

#define S_LOW  TRANSMIT_PORT &= ~_BV(TRANSMIT_PIN)
#define S_HIGH TRANSMIT_PORT |=  _BV(TRANSMIT_PIN)

static volatile bool state = false;
static volatile uint8_t timer = 0;
static volatile bool transmitOn;

class Remote {
public:
	Remote( uint8_t frequency );
	void send( uint32_t data );
	void test();

private:
	uint8_t mfrequency;

	void init();
	void sendHeader();
	void sendPause();
	void sendOne();
	void sendZero();
};

#endif
