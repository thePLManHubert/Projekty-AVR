/* Pojedynczy modu³ pod³¹czany do kontrolera g³ównego za pomoc¹ magistrali RS485 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "LCD/lcd.h"
#include "SoftwareUART/SoftwareUART.h"
#include <util/delay.h>

#define RS485_DDR  		DDRB
#define RS485_PORT 		PORTB
#define RS485_PIN  		PORTB1

#define CONTROLLER_DDR  DDRA
#define CONTROLLER_PORT PORTA
#define CONTROLLER_PIN  PORTA0
#define CONTROLLER_ON   CONTROLLER_PORT |= _BV(CONTROLLER_PIN)
#define CONTROLLER_OFF  CONTROLLER_PORT &= ~_BV(CONTROLLER_PIN)

#define TURN_OFF_PACKAGE 	'0'
#define TURN_ON_PACKAGE 	'1'
#define INFO_PACKAGE 		'2'
#define RESET_TIME_PACKAGE 	'3'

#define RECEIVER_IDX 		0
#define SENDER_IDX 			1
#define MESSAGE_TYPE_IDX 	2
#define TIME_L_IDX 			3
#define TIME_H_IDX 			4

#define STATE_OFF 	0
#define STATE_ON 	1

#define ADDRESS 		48			// indywidualny adres kontrolera
#define PACKAGE_SIZE	5			// rozmiar paczki przesy³anej po lini RS485

/*-----------------------------------------------------------------------------------------*/

void onReceivePackageRS485( uint8_t data );
void timeoutCallback();
void processData( uint8_t * data );

uint8_t bufferIn[10];				// bufor odbioru
uint8_t bufferOut[5];				// bufor wyjœciowy
volatile uint8_t bufferIndex = 0;	// indeks odbieranego aktualnie bajtu
volatile uint8_t readyToSend = 0;	// informuje o gotowoœci do wys³ania danych
volatile uint8_t state = STATE_OFF;	// zawiera aktualny stam urz¹dzenia (w³/wy³)
volatile uint8_t sec;				// daje znac czy up³ynê³a sekunda pracy
volatile uint16_t timeOn = 0;		// zawiera czas w którym urz¹dzenie by³o w³¹czone

Lcd lcd = Lcd();

int main( void ) {
	SoftwareUART::begin( 9600 );
	SoftwareUART::setCallback( &onReceivePackageRS485 );
	SoftwareUART::setTimeout( 10, &timeoutCallback );
	sei();

	CONTROLLER_DDR |= _BV( CONTROLLER_PIN );
	RS485_DDR |= _BV( RS485_PIN );

	while (1) {
		if (readyToSend) {
			_delay_ms(1);
			SoftwareUART::sendBytes( bufferOut, PACKAGE_SIZE );
			readyToSend = 0;
			bufferIndex = 0;
		}
		if (sec == 1) {
			sec = 0;
			timeOn++;
		}
	}
}

void onReceivePackageRS485( uint8_t data ) {
	bufferIn[bufferIndex++] = data;
	if (bufferIndex == 5) {
		SoftwareUART::stopCounting();

		if (bufferIn[0] == ADDRESS) {
			processData( bufferIn );
		}
	}
}

void timeoutCallback() {
	bufferIndex = 0;
	lcd.clear();
	lcd.print( "Timeout." );

	SoftwareUART::stopCounting();
}

void constructPackage( uint8_t * data ) {
	bufferOut[RECEIVER_IDX] = 0;
	bufferOut[SENDER_IDX] = ADDRESS;
	bufferOut[MESSAGE_TYPE_IDX] = (state) ? TURN_ON_PACKAGE : TURN_OFF_PACKAGE;
	bufferOut[TIME_L_IDX] = ADDRESS+state;
	bufferOut[TIME_H_IDX] = 0;
}

void processData( uint8_t * data ) {
	switch (data[MESSAGE_TYPE_IDX]) {
	case TURN_OFF_PACKAGE:
		state = STATE_OFF;
		CONTROLLER_OFF;
		constructPackage( data );
		readyToSend = 1;
	break;
	case TURN_ON_PACKAGE:
		state = STATE_ON;
		CONTROLLER_ON;
		constructPackage( data );
		readyToSend = 1;
	break;
	case INFO_PACKAGE:
		constructPackage( data );
		readyToSend = 1;
	break;
	case RESET_TIME_PACKAGE:
		timeOn = 0;
		constructPackage( data );
		readyToSend = 1;
	break;
	}
}
