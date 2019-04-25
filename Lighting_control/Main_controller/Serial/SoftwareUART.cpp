/* Programowy UART do komunikacji kontrolerów oœwietlenia z mikrokontrolerem centralnym */

#include "SoftwareUART.h"
#include "../Definitions.h"
#include <avr/interrupt.h>
#include "../utilities/Utilities.h"
#include <avr/io.h>

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
//volatile extern uint8_t transmitted;

DataReceivedCallback SoftwareUART::callback;
TimeoutCallback SoftwareUART::tcallback;
uint16_t SoftwareUART::timeOutLength;
volatile uint8_t SoftwareUART::isCounting;
volatile uint16_t SoftwareUART::counter;


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
	TCCR1B = _BV(WGM12) | _BV( CS10 ); //tryb CTC, brak podzia³u czêstotliwoœci
	TIMSK1 = _BV( OCIE1A ); //przerwanie dla Output Compare
	OCR1A = F_CPU / baudRate / 3 - 1; //okres licznika
	DDR_TX = _BV( PIN_TX ); //pin wybrany jako TX konfigurowany jako wyjœcie
	PORT_TX |= _BV( PIN_TX ); //TX domyœlnie w stanie wysokim
}

void SoftwareUART::setTimeout( uint16_t millis, TimeoutCallback tcall) {
	tcallback = tcall;
	timeOutLength = 5000;
	if(millis <= 32000)
		timeOutLength = millis;
	TCCR2A |= _BV(COM2A1) | _BV( WGM21 );		// tryb CTC
	TIMSK2 |= _BV( OCIE2A );					// w³¹cza przerwanie
	TCCR2B |= _BV(CS22); 						// preskaler 64
	OCR2A = 249;								// przerwanie co 1ms
}

void SoftwareUART::startCounting(){
	isCounting = 1;
}

void SoftwareUART::restartCounting(){
	counter = 0;
}

void SoftwareUART::stopCounting(){
	isCounting = 0;
	counter = 0;
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
	_delay_us(750);
	RS485_RECEIVE;
}

void SoftwareUART::sendString( char str[] ) {
	uint16_t i = 0;
	while (str[i]) {
		sendByte( str[i++] );
	}
}

void SoftwareUART::executeTimeoutCallback(){
	tcallback();
}

ISR(TIMER1_COMPA_vect) {
	static uint8_t bitsToReceive;
	static uint8_t inData;

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
				SoftwareUART::restartCounting();
				if (SoftwareUART::isCallbackNotNull()) {
					SoftwareUART::executeCallback( softwareUartInputData );
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
