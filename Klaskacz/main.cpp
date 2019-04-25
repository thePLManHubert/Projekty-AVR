/* Program do urz¹dzenia steruj¹cego w³¹czaniem i wy³¹czaniem urz¹dzeñ pod³¹czonych do niego.
   Sterowanie odbywa siê za pomoc¹ dŸwiêku - podwójne klaœniêcie w³¹cza/wy³¹cza urz¹dzenie */

#include <avr/io.h>
#include <avr/interrupt.h>

//ATtiny13A //10.08.2017 hubi92
#define START_TIME 106 //przerwanie timera co 1ms
#define READ 2 //pin 7
#define OUT 3 //pin 2

int state = 0;
int time = 0;
int level = 0;
int counter = 0;


int main( void ){

	DDRB |= (1<<OUT) & ~(1<<READ);
	sei();
	//TCCR0A = 0;
	//TCCR0B = 0;
	TCCR0B |= (1<<CS01) | (1<<CS00); //prescaler 64
	TIMSK0 |= (1<<TOIE0); //przerwania timera wlaczone
	TCNT0 = START_TIME;

	while(true) {

	  if(PINB & (1<<READ)){
	    level = 1;
	    counter = 0;
	  }

	  switch(state){
	  case 0:
	    if(level){
	      time = 0; //zeruj czas (tylko na poczatku sekwencji)
	      state = 1; //kolejny stan
	    }
	  break;

	  case 1:
	    if(!level){
	      if(time < 150){ //100ms zapasu bo sygnal wysoki trwa 50ms
	                      //w razie gdyby sygnal wysoki utrzymywal sie zbyt dlugo
	        state = 2; //kolejny stan
	      }
	      else state = 0; //powrot do poczatku
	    }
	  break;

	  case 2:
	    if(level){
	      if(time > 150 && time < 650){ //min czas miedzy klasnieciami to 50ms, max to 650ms
	        PORTB ^= (1<<OUT); //zmiana stanu urzadzenia
	        state = 3; //kolejny stan
	      }//przy braku stanu 3 je¿eli po drugim klasnieciu wystapi trzecie stosunkowo szybko (do ok. 500ms) to przelaczenie wystapi ponownie
	      else state = 0; //powrot do poczatku
	    }
	  break;

	  case 3:
	    if(time > 1000) state = 0; //350ms - 850ms odstepu miedzy sekwencjami
	  break;

	  }
	}
}


ISR(TIM0_OVF_vect){ //przerwania timera co 1ms
  TCNT0 = START_TIME;

  if(level == 1 && counter == 50){
    level = 0;
  }

  time++; counter++;
}

