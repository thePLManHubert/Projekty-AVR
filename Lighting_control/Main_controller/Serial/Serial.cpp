/* Komunikacja g³ównego mikrokontrolera z komputerem */

#include "Serial.h"
#include "../utilities/Utilities.h"
#include "../Definitions.h"
#include <avr/interrupt.h>

extern uint8_t bufferInRS232[8];
extern volatile uint8_t bufferIndexRS232;
onRS232Receive HardwareSerial::RS232CB;
volatile uint8_t HardwareSerial::isCounting;
volatile uint16_t HardwareSerial::counter;

void HardwareSerial::begin( uint16_t baudRate ) {
	uint16_t ubrr = __UBRR( baudRate );

	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)ubrr;

	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // w³¹cz nadajnik i odbiornik
	UCSR0B |= _BV( RXCIE0 );

	//ustawianie timera opóŸniaj¹cego
	TCCR2A |= _BV(COM2A1) | _BV( WGM21 );	// CTC dla COMPA
	TIMSK2 |= _BV( OCIE2A );				// w³¹czenie przerwania
	TCCR2B |= _BV( CS22 ); 					// preskaler 64
	OCR2A = 249;							// ustawienie wartoœci szczytowej (250 taktów)
}

void HardwareSerial::sendByte( uint8_t value ) {
	while (!( UCSR0A & (1 << UDRE0)))
		;
	UDR0 = value;
}

uint8_t HardwareSerial::sendBytes( uint8_t * buffer, uint8_t size ) {
	uint8_t howMany = 0;
	for (int i = 0; i < size; i++) {
		sendByte( buffer[i] );
		howMany++;
	}
	return howMany;
}

void HardwareSerial::sendString( char * s ) {
	while (*s)
		sendByte( *s++ );
}

void HardwareSerial::setCallback( onRS232Receive call ) {
	RS232CB = call;
}

void HardwareSerial::executeCallback() {
	RS232CB();
}

bool HardwareSerial::isCallbackNotNull() {
	return RS232CB != 0;
}

void HardwareSerial::startCounting() {
	isCounting = 1;
}

void HardwareSerial::restartCounting() {
	counter = 0;
}

void HardwareSerial::stopCounting() {
	isCounting = 0;
	counter = 0;
}

ISR( USART_RX_vect ) {
	bufferInRS232[bufferIndexRS232] = UDR0;
	HardwareSerial::startCounting();
	HardwareSerial::restartCounting();

	if (++bufferIndexRS232 == PACKAGE_SIZE_RS232) {
		if (HardwareSerial::isCallbackNotNull()) {
			HardwareSerial::executeCallback();
		}
	}
}

