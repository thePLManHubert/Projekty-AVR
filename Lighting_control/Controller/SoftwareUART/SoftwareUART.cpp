#include "SoftwareUART.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#define RS485_DDR  		DDRB
#define RS485_PORT 		PORTB
#define RS485_PIN  		PORTB1
#define RS485_TRANSMIT  RS485_PORT |= _BV(RS485_PIN)
#define RS485_RECEIVE   RS485_PORT &= ~_BV(RS485_PIN)

#define STATE_OFF 	0
#define STATE_ON 	1

enum SoftwareUartStateRx {
	IDLE_RX, RECEIVE, RECEIVE_STOP
};

enum SoftwareUartStateTx {
	IDLE_TX, TRANSMIT, TRANSMIT_STOP
};

volatile enum SoftwareUartStateRx softwareUartStateRx = IDLE_RX;
volatile enum SoftwareUartStateTx softwareUartStateTx = IDLE_TX;
volatile uint8_t cyclesCounterRx, cyclesCounterTx; //liczba przerwañ w bicie
volatile uint8_t softwareUartBitsToSend; //pozosta³e bity do wys³ania
volatile uint16_t softwareUartOutputData; //wysy³any bajt
volatile uint8_t softwareUartInputData; //odbierany bajt
volatile uint8_t softwareUartNewData = 0; //flaga odebranego bajtu

DataReceivedCallback SoftwareUART::callback;
TimeoutCallback SoftwareUART::tcallback;
volatile uint16_t SoftwareUART::counter;
uint16_t SoftwareUART::timeOutLength;
static uint16_t currentTime;
extern uint8_t sec;
extern uint8_t state;

void SoftwareUART::setCallback( DataReceivedCallback call ) {
	callback = call;
}

void SoftwareUART::executeCallback( uint8_t data ) {
	callback( data );
}

bool SoftwareUART::isCallbackNotNull() {
	return callback != 0;
}

void SoftwareUART::begin( uint16_t baudRate ) {
	TCCR1B = _BV(WGM12) | _BV( CS10 ); 		//tryb CTC, brak podzia³u czêstotliwoœci
	TIMSK1 = _BV( OCIE1A ); 				//przerwanie dla Output Compare
	OCR1A = F_CPU / baudRate / 3 - 1; 		//okres licznika
	DDR_TX = _BV( PIN_TX ); 				//pin wybrany jako TX konfigurowany jako wyjœcie
	PORT_TX |= _BV( PIN_TX ); 				//TX domyœlnie w stanie wysokim
}

void SoftwareUART::setTimeout( uint16_t millis, TimeoutCallback tcall ) {
	tcallback = tcall;
	timeOutLength = 5000;
	if (millis <= 32000)
		timeOutLength = millis;
	TCCR0A |= _BV( WGM01 );					// tryb CTC
	TIMSK0 |= _BV( OCIE0A );				// w³¹cza przerwanie
	OCR0A = 124;							// przerwanie co 1ms
}

void SoftwareUART::startCounting() {
	TCCR0B |= _BV(CS01) | _BV( CS00 ); 		// preskaler 64
}

void SoftwareUART::restartCounting() {
	TCNT0 = 0;
	counter = 0;
}

void SoftwareUART::stopCounting() {
	TCCR0B &= ~_BV( CS01 ) & ~_BV( CS00 ); 	// przestañ odmierzac czas
	TCNT0 = 0;								// zeruj timer
	counter = 0;							// zeruj timer
}

void SoftwareUART::sendByte( uint8_t data ) {
	while (softwareUartStateTx != IDLE_TX)
		;
	cyclesCounterTx = 1;
	softwareUartBitsToSend = 10;
	softwareUartOutputData = ((uint16_t)data << 1) | 0b1000000000;
	softwareUartStateTx = TRANSMIT;
}

void SoftwareUART::sendBytes( uint8_t data[], uint16_t size ) {
	RS485_TRANSMIT;
	for (uint16_t i = 0; i < size; i++) {
		sendByte( data[i] );
	}
	_delay_us(450);
	RS485_RECEIVE;
}

void SoftwareUART::sendString( char str[] ) {
	uint16_t i = 0;
	while (str[i]) {
		sendByte( str[i++] );
	}
}

void SoftwareUART::executeTimeoutCallback() {
	tcallback();
}

ISR(TIM0_COMPA_vect) {
	SoftwareUART::counter++;
	if (SoftwareUART::counter >= SoftwareUART::timeOutLength) {
		SoftwareUART::executeTimeoutCallback();
	}
}

ISR(TIM1_COMPA_vect) {
	static uint8_t bitsToReceive;
	static uint8_t inData;

	if (state == STATE_ON) {
		if (++currentTime == 28880) {
			currentTime = 0;
			sec = 1;
		}
	}

	if (softwareUartStateTx == TRANSMIT) {
		if (--cyclesCounterTx == 0) {
			if (softwareUartOutputData & 1)
				PORT_TX |= _BV( PIN_TX );
			else PORT_TX &= ~_BV( PIN_TX ); //data bit
			softwareUartOutputData >>= 1;
			cyclesCounterTx = 3;
			if (--softwareUartBitsToSend == 0) {
				softwareUartStateTx = TRANSMIT_STOP;
			}
		}
	}

	if (softwareUartStateTx == TRANSMIT_STOP) {
		if (--cyclesCounterTx == 0) {
			softwareUartStateTx = IDLE_TX;
		}
	}

	if ((softwareUartStateRx == IDLE_RX) && !(PORT_RX & _BV( PIN_RX ))) {
		cyclesCounterRx = 4;
		softwareUartStateRx = RECEIVE;
		bitsToReceive = 8;
		inData = 0;
		SoftwareUART::startCounting();
	}

	else if (softwareUartStateRx == RECEIVE) {
		if (--cyclesCounterRx == 0) {
			inData >>= 1;
			if (PORT_RX & _BV( PIN_RX ))
				inData |= 0x80;
			cyclesCounterRx = 3;
			if (--bitsToReceive == 0) {
				softwareUartStateRx = RECEIVE_STOP;
				softwareUartInputData = inData;
				softwareUartNewData = 1;
				if (SoftwareUART::isCallbackNotNull()) {
					SoftwareUART::restartCounting();
					SoftwareUART::executeCallback( inData );
				}
			}
		}
	}

	if (softwareUartStateRx == RECEIVE_STOP) {
		if (--cyclesCounterRx == 0) {
			softwareUartStateRx = IDLE_RX;
		}
	}
}
