/* Program odbierający dane z portu szeregowego komputera, zawierające informacje
 o tym w jakim kolorze powinny świecić dołączone do mikrokontrolera diody LED.  */

#include <avr/io.h>
#include <avr/interrupt.h>

#define FOSC 16000000ULL // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

// R = 220 [ohm]
// G = 220 [ohm]
// B = 440 [ohm]
// anode = 100 [ohm]

//Globals
unsigned char colors[4] = {0,0,0,255};
enum Color {Red = 0, Green, Blue, Modulo};
uint8_t timer = 0;
uint8_t counter = 0;

//Pins
uint8_t R=2, G=3, B=4;

// functions
void USART_Init( unsigned int ubrr);

int main(void){

  DDRC |= _BV(R) | _BV(G) | _BV(B);
  DDRD |= _BV(PD3);
  USART_Init ( MYUBRR );
  sei();

  while(1){

    if(colors[Red] > timer){
      PORTC |= _BV(R);
    }
    else PORTC &= ~_BV(R);

    if(colors[Green] > timer){
      PORTC |= _BV(G);
    }
    else PORTC &= ~_BV(G);

    if(colors[Blue] > timer){
      PORTC |= _BV(B);
    }
    else PORTC &= ~_BV(B);

    timer = (timer+1)%colors[Modulo];

  }

}

void USART_Init( unsigned int ubrr) {
  /* Set baud rate */
  UBRRH = (unsigned char)(ubrr>>8);
  UBRRL = (unsigned char)ubrr;
  /* Enable receiver */
  UCSRB = (1<<RXEN);
  /* Set frame format: 8data, 1stop bit */
  UCSRC = (1<<URSEL)|(3<<UCSZ0);
}

ISR( USART_RXC_vect ) {
	colors[counter] = UDR;
	if(++counter == 4)
		counter = 0;
}
