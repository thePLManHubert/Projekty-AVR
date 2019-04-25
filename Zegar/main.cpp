#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "Zegar.h"

Zegar zegar = Zegar();
bool justUsed = false; // some button
bool pressed_hrs_button = false;
bool pressed_min_button = false;

int main(void){

	TCCR1B |= (1<<CS11) | (1<<CS10) | (1<<WGM12); // timer 1 w³¹czony preskaler 64 CTC
	OCR1A = 625; // 40ms opóŸnienia dla drgañ styków
	TIMSK |= (1<<OCIE1A); // przerwanie timer 1 w³¹czone
	sei();
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);

	PORTC = 54; // input pullup PC1,2,4,5

	zegar.start();


	while(1){

		// praca zegara //

		zegar.work();


		// obs³uga klawiszy //

// godziny
		if(!(PINC & _BV(PINC1))){
			if(!pressed_hrs_button){
				pressed_hrs_button = true;
				if(!justUsed){ // pressed // drgania styków
					justUsed = true;
					TCNT1 = 0;
					zegar.increment_hours();
				}
			}
		} else {
			if(pressed_hrs_button){
				justUsed = true; // released
				TCNT1 = 0;
			}
			pressed_hrs_button = false;
		}

// minuty
		if(!(PINC & _BV(PINC2))){
			if(!pressed_min_button){
				pressed_min_button = true;
				if(!justUsed){
					justUsed = true;
					TCNT1 = 0;
					zegar.increment_minutes();
				}
			}
		} else {
			if(pressed_min_button){
				justUsed = true;
				TCNT1 = 0;
			}
			pressed_min_button = false;
		}

// widok
		/*if(PINC & _BV(PINC4)) zegar.set_mode(M_S);
		else zegar.set_mode(H_M);*/

// oszczêdzanie energii
		if(PINC & _BV(PINC0)){
			zegar.set_display(true);
		}
		else {
			zegar.set_display(false);
			sleep_mode();
		}
	}

}

ISR(TIMER2_OVF_vect){

	zegar.on_pulse();

}

ISR(TIMER1_COMPA_vect){

	justUsed = false;

}
