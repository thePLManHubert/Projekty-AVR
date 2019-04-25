/* Prosty programator kości HCS200, wykorzystywanych w pilotach radiowych do sterowania bramami wjazdowymi,
   garażowymi, drzwiami samochodowymi, itd. */

// ATtiny13

#include <avr/io.h>
#include "HCS200_programmer.h"
/*
 * pilot 1
 * 0xABCD1234UL
 *
 * pilot 2
 * 0x732C8E1AUL
 *
 * pilot 3
 * 0xABCD3C18
 *
 * */

int main(void){

	data d = {	0x89F4C76ABD75C14FULL,
							0x0000U,
						0xABCD3C18UL,
						    0x0000UL,
							0x0000U  };
	uint16_t buff[12];
	package p = pack( buff, &d );


	PROGRAM_PORT |= _BV(PB0); 		// starting button
	PROGRAM_PORT_DIR |= _BV(DDB3); 	// veryfication diode

	while(1){

		if(!(PROGRAM_PORT_READ & _BV(PINB0))){
			PROGRAM_PORT &= ~_BV(PB3);
			load(p);
			if(verify(p))
				PROGRAM_PORT |= _BV(PB3);
		}

	}

}
