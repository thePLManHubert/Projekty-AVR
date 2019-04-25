/* Dekoder danych odbieranych z pilota radiowego wyposa¿onego w uk³ad HCS200 */

#include <avr/io.h>
#include <util/delay.h>
#include "HCS200_decoder.h"
#include "Keeloq.h"
#include "Database.h"

//#define DEBUG

#ifdef DEBUG

void fun( HCS200data *result );
void reverseBits( uint32_t *number );
void fun2();
void fun3( HCS200data *result );

#endif



int main(void){

	Database db = Database();
	Receiver r = Receiver();
	HCS200data result;

	DDRA |= _BV(PA0) | _BV(PA1) | _BV(PA2) | _BV(PA3);


	while(1){

		if(r.receive( &result )){

			uint16_t serial = r.getSerialNumber(&result);

			if(db.hasRemote(serial)){

				r.decode(&result);

				uint16_t diff = result.sync - db[serial]->sync;
				if(diff > 0 && diff < 16){
					{	//execute command

						PORTA |= _BV(PORTA1);
						_delay_ms(400);
						PORTA &= ~_BV(PORTA1);

					}
					db[serial]->updateSync(result.sync);
				}
				else if(diff > 0 && diff < 32768){
					db[serial]->updateSync(result.sync);
				}
				else if(diff == 0){// przypadek gdy trzymamy wciœniêty przycisk
					//PORTA ^= _BV(PORTA2);
				}

			}

			r.resume();
		}

	}

}

#ifdef DEBUG
// przeklej do funkcji main
uint32_t sn = r.getSerialNumber( &result );

if(sn == 0xBCD1234UL)
	PORTA ^= _BV(PA1);
else if(sn == 0x2C42DB1)
	PORTA ^= _BV(PA2);
else if(sn == 0)
	PORTA ^= _BV(PA3);
else PORTA ^= _BV(PA4);
//koniec treœci do przeklejenia

// for testing
void fun(HCS200data *result){

	uint8_t *pointer = (uint8_t*) result;

	for(int i = 3; i >= 0; i--){

		PORTD = *(pointer+i);

		_delay_ms(2000);
	}

}

void fun2(){

	uint32_t num = 0x12AB5477;
	PORTD = num;
	uint8_t *p = (uint8_t*) &num;

	for(int i = 0; i < 4; i++){
		PORTD = *(p+i);
		_delay_ms(5000);
	}

	reverseBits( &num );

	for(int i = 0; i < 4; i++){
		PORTD = *(p+i);
		_delay_ms(5000);
	}

}

void fun3( HCS200data *result ){

	PORTD = result->sync >> 8;
	_delay_ms(5000);
	PORTD = result->sync;
	_delay_ms(5000);
	PORTD = result->discrimination_bits >> 8;
	_delay_ms(5000);
	PORTD = result->discrimination_bits;
	_delay_ms(5000);
	PORTD = result->serial_number >> 24;
	_delay_ms(5000);
	PORTD = result->serial_number >> 16;
	_delay_ms(5000);
	PORTD = result->serial_number >> 8;
	_delay_ms(5000);
	PORTD = result->serial_number;
	_delay_ms(5000);
	PORTD = result->buttons;
	_delay_ms(5000);
	PORTD = result->battery_low;
	_delay_ms(5000);
	PORTD = result->RPT;

}

void reverseBits( uint32_t *number ){

	uint8_t *d = (uint8_t*) number;

	for ( int8_t _byte = 0; _byte < 4 ; _byte++ ){

		uint8_t tempByte = 0;

		for (int8_t _bit = 0; _bit < 7; _bit++){

			tempByte |= !!(d[_byte] & _BV(_bit));
			tempByte <<= 1;

		}
		tempByte |= !!(d[_byte] & _BV(7));
		d[_byte] = tempByte;

	}
}

#endif
