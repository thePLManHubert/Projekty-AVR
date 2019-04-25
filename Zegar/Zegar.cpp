#include <avr/io.h>
#include "Zegar.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define with_dot_on true
#define with_dot_off false
bool refresh = true; // czy odœwie¿yc wyswietlacz

const uint8_t without_dot[11] PROGMEM = { 192, 249, 164, 176, 153, 146, 130, 248, 128, 144, 255 };
const uint8_t with_dot[11]    PROGMEM = {  64, 121,  36,  48,  25,  18,   2, 120,   0,  16, 255 };

Zegar::Zegar() : j(0), d(0), s(0), t(0),
				 secs(0), mins(0), hours(0),
				 core(0), position(4), m(H_M),
				 ticking(false), display(true), pulse(false)
{
	init();
}

Zegar::Zegar(uint8_t h, uint8_t min, uint8_t sec)
	: secs(sec), mins(min), hours(h),
	  core(0), position(4), m(H_M),
	  ticking(false), display(true), pulse(false)
{
	take_hours_mins();
	init();
}

void Zegar::init() {

  DDRD = 255; //wyjscia dla segmentow PD0 = a, PD1 = b, itd.
  PORTD = 255; //segmety wylaczone
  DDRB = 15; //wyjscia dla cyfr PB0 = j, PB1 = s, itd.
  PORTB = 0; //cyfry wylaczone

  ASSR |= (1<<AS2); // tryb asynchroniczny dla rezonatora zegarkowego
  TCCR2 |= (1<<CS22); //w³¹cz zegar globalny z prescalerem 64
  TCCR0 |= (1<<CS01); // timer 0 z preskalerem 8 (odœwie¿anie wyœwietlacza 488Hz)
  TIMSK |= (1<<TOIE0) | (1<<TOIE2); //przerwania zegara

}

void Zegar::work(){

	if(ticking){ // je¿eli zegar w³¹czony
		if(pulse){ // je¿eli nast¹pi³o przepe³nienie globalnego timera zegarowego
			pulse = !pulse;
			if(++core == COMPARE){ // je¿eli minê³a sekunda
				tick(); //odmierzanie czasu (sek++)
				refresh_digits(); // ustawienie t,s,d,j
				core = 0;
			}
		}
	}

	if(display){
		if(refresh){
			refresh = !refresh;
			if(core < COMPARE/2) show(with_dot_on); //pokazywanie cyfr (t,s,d,j) na wyswietlaczu (z kropk¹)
			else show(with_dot_off); //pokazywanie cyfr na wyswietlaczu (bez kropki)
		}
	}

}

void Zegar::start(){

	ticking = true;

}

void Zegar::stop(){

	ticking = false;

}

void Zegar::reset(){

	hours = mins = secs = 0;

	refresh_digits();

}

void Zegar::increment_seconds(){

	if(secs < 59)
		secs++;
	else secs = 0;

	refresh_digits();

}

void Zegar::increment_minutes(){

	if(mins < 59)
		mins++;
	else mins = 0;

	refresh_digits();

}

void Zegar::increment_hours(){

	if(hours < 23)
		hours++;
	else hours = 0;

	refresh_digits();

}

void Zegar::tick(){

	secs++;
	if(secs == 60){
		secs = 0; mins++;
		if(mins == 60){
			mins = 0; hours++;
			if(hours == 24){
				hours = 0;
			}
		}
	}

}

void Zegar::show(bool dot_on){

	position--; //przejdz do nizszej cyfry

	switch(position){
	case 3:
		PORTB &= ~(1<<PB0); //wylacz poprzednia cyfre wyswietlacza
		PORTD = without_dot[t]; //zmien zawartosc dla cyfry
		PORTB |= (1<<PB3); //wlacz nastepna cyfre wyswietlacza
	break;
	case 2:
		PORTB &= ~(1<<PB3);
		PORTD = (dot_on) ? with_dot[s] : without_dot[s];
		PORTB |= (1<<PB2);
	break;
	case 1:
		PORTB &= ~(1<<PB2);
		PORTD = without_dot[d];
		PORTB |= (1<<PB1);
	break;
	case 0:
		PORTB &= ~(1<<PB1);
		PORTD = without_dot[j];
		PORTB |= (1<<PB0);
		position = 4; //zapetlenie cyklu
	break;
	}

}

void Zegar::take_hours_mins(){

	t = hours / 10;
	s = hours % 10;
	d = mins / 10;
	j = mins % 10;

}

void Zegar::take_mins_secs(){

	t = mins / 10;
	s = mins % 10;
	d = secs / 10;
	j = secs % 10;

}

void Zegar::set_display(bool value){

	if(!(display = value))
		PORTB = 0;

}

void Zegar::on_pulse(){

	pulse = true;

}

void Zegar::refresh_digits(){

	if(m == H_M) take_hours_mins();
	else if(m == M_S) take_mins_secs();

}

void Zegar::set_mode(mode m){

	this->m = m;
	refresh_digits();

}

ISR(TIMER0_OVF_vect){
	refresh = true;
}
