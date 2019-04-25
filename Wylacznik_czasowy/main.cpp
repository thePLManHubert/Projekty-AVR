/* Projekt wy³¹cznika czasowego sterowanego pilorem IR */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdlib.h>
#include "lcd/lcd.h"

#define LED_PORT_DDR DDRB
#define LED_PORT PORTB
#define LED_PIN PORTB1

#define BACKLIGHT_PORT_DDR DDRB
#define BACKLIGHT_PORT PORTB
#define BACKLIGHT_PIN PORTB4

#define OUTPUT_PORT_DDR DDRD
#define OUTPUT_PORT PORTD
#define OUTPUT_PIN PORTD2

//#define USART
//#define READER
#define RECV

#ifdef USART
#define UART_BAUD 9600
#define __UBRR ((F_CPU+UART_BAUD*8UL) / (16UL*UART_BAUD)-1)

void USART_Init( uint16_t ubrr );
void USART_Transmit( uint8_t data );
void USART_Send_S( char * s );
void USART_Send_V( int8_t value );

bool writeToUSART = false;
#endif

void addTime( int16_t sec );
void subtractTime( int16_t sec );
void showTime();
void propSignalRecv();

Lcd lcd;
volatile bool writeToLCD = false;

enum stage {
	start, header, pause, bits
};

volatile uint8_t bitNumber = 0;
volatile uint32_t packet = 0;
volatile bool newPacket = 0;
stage state = start;

volatile bool timerOn = false;
volatile int16_t sec = 0;
volatile uint8_t ticks = 0;

uint8_t brightness = 100;
uint8_t receiveLED = 0;
uint8_t backlightTimer = 0;
uint8_t backlightLED = 5; // 5 sec podœwietlenia
int16_t maxTime = 60 * 60 * 2;	// 2 godziny

int main() {

	LED_PORT_DDR |= _BV(LED_PIN);
	BACKLIGHT_PORT_DDR |= _BV(BACKLIGHT_PIN);
	OUTPUT_PORT_DDR |= _BV(OUTPUT_PIN);

#ifdef USART
	USART_Init( __UBRR );
#endif

	//preskaler 256, taktowanie 62500 Hz, tryb CTC
	TCCR2 |= _BV(CS22) | _BV( CS21 ) | _BV( WGM21 );
	//reset timera co ok. 4 ms
	OCR2 = 249;

	//preskaler 64, taktowanie 250000 Hz, tryb CTC
	TCCR1B = _BV(CS11) | _BV( CS10 ) | _BV( WGM12 );
	//reset timera co ok. 100 ms
	OCR1A = 25000;
	//przerwania dla input capture i output compare
	TIMSK = _BV(TICIE1) | _BV( OCIE1A ) | _BV( OCIE2 );
	sei();
	//set_sleep_mode( SLEEP_MODE_IDLE );
	//sleep_enable();

	lcd = Lcd();
	BACKLIGHT_PORT |= _BV(BACKLIGHT_PIN); // w³¹cz podœwietlenie

	while (1) {
		//sleep_cpu();

#ifdef USART
		if(writeToUSART) {
			writeToUSART = false;
			USART_Send_S( "Pozostalo: " );
			USART_Send_V( sec / 60 );
			USART_Send_S( " minut i " );
			USART_Send_V( sec % 60 );
			USART_Send_S( " sekund\r\n" );
		}
#endif
		if (writeToLCD) {
			writeToLCD = false;
			showTime();
		}

		// warunek wy³¹czenie urz¹dzenia
		if (timerOn && sec == 0) {
			timerOn = false;
			writeToLCD = false; // make sure the screen is cleared at the end of counting
			OUTPUT_PORT &= ~_BV(OUTPUT_PIN);
			lcd.clear();
		}

		//w³¹cz podœwietlenie przed koñcem odliczania
		if (sec == 20) {
			backlightLED = 20;
			BACKLIGHT_PORT |= _BV(BACKLIGHT_PIN);
		}
#ifdef READER
		if (newPacket) {
			PORTB |= _BV( 1 );
			receiveLED = brightness;
			backlightLED = 10;
			PORTB |= _BV( 3 );

			newPacket = false;
			PORTC |= _BV( 0 );
			receiveLED = brightness;
			lcd.clear();
			lcd.print( "Packet:" );
			lcd.goToXY( 0, 1 );

			char num[16];
			int high = packet >> 16;
			int low = packet;
			itoa( high, num, 16 );
			if ((high & (0xf << 12)) == 0)
				lcd.print( "0" );
			if ((high & (0xff << 8)) == 0)
				lcd.print( "0" );
			lcd.print( num );
			itoa( low, num, 16 );
			if ((low & (0xf << 12)) == 0)
				lcd.print( "0" );
			if ((low & (0xff << 8)) == 0)
				lcd.print( "0" );
			lcd.print( num );
		}
#endif
#ifdef RECV
		if (newPacket) {
			newPacket = 0;

			switch (packet) {
				case 0xFF00FF:
				propSignalRecv();
				OUTPUT_PORT ^= _BV(OUTPUT_PIN);
				break;
				case 0xFFC03F:
				propSignalRecv();
				timerOn = !timerOn;
				if (timerOn) {
					addTime( 60 );
				} else {
					sec = 0;
					ticks = 0;
					lcd.clear();
				}
				break;
				case 0xFF30CF:	// prze³¹cz podœwietlenie
				propSignalRecv();
				BACKLIGHT_PORT ^= _BV(BACKLIGHT_PIN);
				break;
				case 0xFF10EF:
				propSignalRecv();
				addTime( 60 );
				break;
				case 0xFF708F:
				propSignalRecv();
				subtractTime( 60 );
				break;
				case 0xFFA05F:
				propSignalRecv();
				addTime( 300 );
				break;
				case 0xFF40BF:
				propSignalRecv();
				subtractTime( 300 );
				break;
				case 0xFFE01F:
				propSignalRecv();
				addTime( 1800 );
				break;
				case 0xFF58A7:
				propSignalRecv();
				subtractTime( 1800 );
				break;
			}
		}
#endif

		// zgaœ diodê odbiorcz¹ i podœwietlenie
		if (--receiveLED == 0)
			LED_PORT &= ~_BV(LED_PIN);
		if (backlightLED == 0)
			BACKLIGHT_PORT &= ~_BV(BACKLIGHT_PIN);
	}
}

void propSignalRecv() {
	// zaœwiec diodê odbiorcz¹
	LED_PORT |= _BV(LED_PIN);
	receiveLED = brightness;

	// w³¹cz podswietlenie
	if (packet != 0xFF00FF) {
		backlightLED = 10;
		if (packet != 0xFF30CF)
			BACKLIGHT_PORT |= _BV(BACKLIGHT_PIN);
	}
}

void addTime( int16_t time ) {
	timerOn = true;
	if (sec <= maxTime - time) {	// max time
		sec += time;
		showTime();
	}
}

void subtractTime( int16_t time ) {
	if (sec - time <= 0) {
		timerOn = false;
		sec = 0;
		ticks = 0;
		lcd.clear();
	} else {
		sec -= time;
		showTime();
	}
}

void showTime() {
	lcd.clear();
	lcd.print( "Do konca" );
	lcd.goToXY( 0, 1 );

	uint8_t h = sec / 3600;
	uint8_t m = (sec / 60) % 60;
	uint8_t s = sec % 60;

	lcd.print( h );
	lcd.print( ":" );
	if (m < 10)
		lcd.print( 0 );
	lcd.print( m );
	lcd.print( ":" );
	if (s < 10)
		lcd.print( 0 );
	lcd.print( s );

}

#ifdef USART
void USART_Init( uint16_t ubrr ) {
	UBRRH = (uint8_t)(ubrr >> 8);
	UBRRL = (uint8_t)ubrr;

	UCSRB = (1 << RXEN) | (1 << TXEN);
}

void USART_Transmit( uint8_t data ) {
	while (!( UCSRA & (1 << UDRE)))
	;

	UDR = data;
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
#endif

ISR(TIMER1_CAPT_vect) {
	TCNT1 = 0;

	switch (state) {
	case start:
		state = header;
		TCCR1B ^= _BV( ICES1 );		// zbocze ^
	break;
	case header:
		if (ICR1 > 2100) {			// 2250
			state = pause;
			TCCR1B ^= _BV( ICES1 );
		}
	break;
	case pause:
		if (ICR1 > 1000) {			// 1075
			state = bits;
			TCCR1B ^= _BV( ICES1 );		// zbocze ^
		}
	break;
	case bits:
		if (!(TCCR1B & _BV( ICES1 ))) { // zbocze opadaj¹ce
			packet <<= 1;
			bitNumber++;
			if (ICR1 < 150)			// 125
				packet &= ~1;
			else if (ICR1 < 420)	// 400
				packet |= 1; // wstaw jedynkê na najm³odszej pozycji
		}

		if (bitNumber == 32) {
			newPacket = true;
			bitNumber = 0;
			state = start;
			TCCR1B |= _BV( ICES1 );
		}

		TCCR1B ^= _BV( ICES1 );
	}
}

ISR(TIMER1_COMPA_vect) {
	TCCR1B &= ~_BV( ICES1 );
	bitNumber = 0;
	state = start;	// wróc do pocz¹tku procedury
	packet = 0;
}

ISR(TIMER2_COMP_vect) {
	if (timerOn) {
		if (++ticks == 250) {
			ticks = 0;
			sec--;
			writeToLCD = true;
#ifdef USART
			writeToUSART = true;
#endif
		}
	}
	if (++backlightTimer == 250) { // zarz¹dzaj czasem podœwietlenia
		backlightTimer = 0;
		backlightLED--;
	}
}
