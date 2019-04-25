#ifndef _SOFTWAREUART_H
#define _SOFTWAREUART_H

#include <avr/io.h>

#define PORT_RX PINB
#define PIN_RX PINB2
#define PORT_TX PORTB
#define PIN_TX PORTB0
#define DDR_TX DDRB

typedef void (*DataReceivedCallback)( uint8_t );
typedef void (*TimeoutCallback)();

class SoftwareUART {
public:
	static void begin( uint16_t baudRate );
	static void sendByte( uint8_t data );
	static void sendBytes( uint8_t data[], uint16_t size );
	static void sendString( char str[] );
	static void setTimeout( uint16_t millis, TimeoutCallback tcall );
	static void startCounting();
	static void restartCounting();
	static void stopCounting();
	static void executeTimeoutCallback();
	static void setCallback( DataReceivedCallback call );
	static void executeCallback( uint8_t data );
	static bool isCallbackNotNull();

	static volatile uint16_t counter;
	static uint16_t timeOutLength;

private:
	static DataReceivedCallback callback;
	static TimeoutCallback tcallback;
};

#endif
