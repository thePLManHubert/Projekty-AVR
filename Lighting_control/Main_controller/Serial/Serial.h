#ifndef _SERIAL_H
#define _SERIAL_H

#include <avr/io.h>

#define __UBRR(x) ((F_CPU+x*8UL) / (16UL*x)-1)

typedef void (*onRS232Receive)( void );

class HardwareSerial {
public:
	static void begin( uint16_t baudRate );

	static void sendByte( uint8_t value );
	static uint8_t sendBytes( uint8_t * buffer, uint8_t size );
	static void sendString( char * s );
	static void setCallback( onRS232Receive call );
	static void executeCallback();
	static bool isCallbackNotNull();
	static void startCounting();
	static void restartCounting();
	static void stopCounting();

	static volatile uint16_t counter;
	static volatile uint8_t isCounting;

private:
	static onRS232Receive RS232CB;
};

#endif
