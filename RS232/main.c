/* Testowanie wbudowanego w mikrokontroler układu UART */

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/interrupt.h>

#define UART_BAUD 1000000L
#define __UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1)

void USART_Init( uint16_t ubrr );
void USART_Send_S( char * s );
void USART_Send_V( int8_t value );
void USART_Transmit( char data );
char USART_Receive( void );
uint8_t getValue();
uint8_t readBytes( uint8_t * buffer, uint8_t size );

//Globals
uint8_t timerOn = 0;
uint8_t pins[14] = { 0 };
uint8_t colors[3];
volatile uint8_t bufferIndex = 0;
enum Color {
	Red = 0, Green, Blue
};
uint8_t timer = 0;

//Pins
uint8_t R = 4, G = 5, B = 7;
const int MAX = 256;

int main( void ) {

	DDRD |= _BV(R) | _BV( B );
	DDRC |= _BV( G );
	USART_Init( __UBRR );
	sei();
	UCSR0B |= _BV( RXCIE0 );

	while (1) {
		if (MAX - colors[Red] > timer) {
			PORTD |= _BV( R );
		} else PORTD &= ~_BV( R );

		if (MAX - colors[Green] > timer) {
			PORTC |= _BV( G );
		} else PORTC &= ~_BV( G );

		if (MAX - colors[Blue] > timer) {
			PORTD |= _BV( B );
		} else PORTD &= ~_BV( B );

		timer++;
	}

}

ISR( USART_RX_vect ) {
	uint8_t value = UDR0;
	colors[bufferIndex] = value;
	bufferIndex = (bufferIndex + 1) % 3;
}

void USART_Init( uint16_t ubrr ) {
	/*Set baud rate */
	UBRR0H = (char)(ubrr >> 8);
	UBRR0L = (char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	/* Set frame format: 8data, 2stop bit */
	//UCSR0C = (1 << USBS0) | (3 << UCSZ00);
}

void USART_Transmit( char data ) {
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)))
		;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

char USART_Receive( void ) {
	/* Wait for data to be received */
	while (!(UCSR0A & (1 << RXC0)))
		;
	/* Get and return received data from buffer */
	return UDR0;
}

void USART_Send_S( char * s ) {
	while (*s)
		USART_Transmit( *s++ );
}

void USART_Send_V( int8_t value ) {
	char num[9];
	itoa( value, num, 10 );
	USART_Send_S( num );
}

uint8_t getValue() {
	uint8_t val = 0;
	char buffer[8] = { 0 };
	char temp;

	while ((temp = USART_Receive()) != '\r') {
		buffer[val++] = temp;
	}
	val = atoi( buffer );
	return val;
}

uint8_t readBytes( uint8_t * buffer, uint8_t size ) {
	uint8_t i;
	for (i = 0; i < size; i++) {
		buffer[i] = USART_Receive();
	}
	return i;
}

