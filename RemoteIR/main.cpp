/* Testowanie odbiornika sygnałów podczerwonych poprzez wysyłanie sygnałów z prostego pilota IR */

#include "Remote.h"
#include <util/delay.h>

#define SIZE 5

Remote remote( 38 );
uint32_t signals[] = { 0xff00fe, 0xff12fd, 0x1108ce, 0x9512de36, 0xff00ff };

int main() {

	DDRD |= _BV( PORTD7 );
	PORTD &= ~_BV( PORTD7 );
	uint8_t i = 0;

	while (1) {
		remote.send( signals[i] );
		_delay_ms( 1000 );
		i = (i + 1) % SIZE;
	}

}
