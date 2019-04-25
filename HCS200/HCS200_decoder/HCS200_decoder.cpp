#include <avr/io.h>
#include "HCS200_decoder.h"
#include "Keeloq.h"

#define TIFR TIFR1 // for ATtiny44
//#define DEBUG



Receiver::Receiver() : stage(start), nHeaderBits(0), nBits(0), dataCaptured(false){
	//OSCCAL = 177;
	TCCR1B = _BV(CS11) | _BV(ICES1) | _BV(WGM12);
	OCR1A = 10000; //should be > HEADER * (UNIT_LENGTH + DEVIATION) // pause between packets (patch 1.01)
}


bool Receiver::receive(HCS200data *buffer){

	if(dataCaptured) return dataCaptured;

	uint8_t * temp = (uint8_t*)buffer;
	bool error = false;

#ifdef DEBUG
	DDRD |= _BV(DDD4);
#endif

	if(TIFR & _BV(OCIE1A)){ // for pause between packets (patch 1.01)
		TIFR |= _BV(OCIE1A);
		error = true;
	}


	switch(stage){

	case start:

		if(TIFR & _BV(ICF1)){
			TIFR |= _BV(ICF1);

			TCCR1B ^= _BV(ICES1);
			TCNT1 = 0;

			stage = preamble;
		}

		break;

	case preamble:

		if(TIFR & _BV(ICF1)){
			TIFR |= _BV(ICF1);

			TCCR1B ^= _BV(ICES1);
			TCNT1 = 0;

			if((ICR1 > UNIT_LENGTH - DEVIATION) && (ICR1 < UNIT_LENGTH + DEVIATION)){
				if(TCCR1B & _BV(ICES1))
					nHeaderBits++;
			}
			else error = true;

		}

		if(nHeaderBits == 12){
			stage = header;
			//TCNT1 = 0; //??
		}

		break;



	case header:

		if(TIFR & _BV(ICF1)){
			TIFR |= _BV(ICF1);

			TCCR1B ^= _BV(ICES1);
			TCNT1 = 0;

			if((ICR1 > HEADER * (UNIT_LENGTH - DEVIATION)) && (ICR1 < HEADER * (UNIT_LENGTH + DEVIATION))){
				stage = data;
			}
			else error = true;
		}

		break;



	case data:

		if(TIFR & _BV(ICF1)){
			TIFR |= _BV(ICF1);

			TCCR1B ^= _BV(ICES1);
			TCNT1 = 0;


			if((ICR1 > ONE * (UNIT_LENGTH - DEVIATION)) && (ICR1 < ONE * (UNIT_LENGTH + DEVIATION))){
				if(TCCR1B & _BV(ICES1)){
					temp[nBits/8] <<= 1;
					temp[nBits/8] |= 1;
					nBits++;
				}
			}
			else if((ICR1 > ZERO * (UNIT_LENGTH - DEVIATION)) && (ICR1 < ZERO * (UNIT_LENGTH + DEVIATION))){
				if(TCCR1B & _BV(ICES1)){
					temp[nBits/8] <<= 1;
					temp[nBits/8] |= 0;
					nBits++;
				}
			}
			else error = true;

		}

		if(nBits == 66){

			bitReverse( buffer );
			dataCaptured = true;

		}

		break;

	}


	if(error){
		resume();
	}

#ifdef DEBUG
	if(TCCR1B & _BV(ICES1))
		PORTD |= _BV(PD4);
	else
		PORTD &= ~_BV(PD4);
#endif

	return dataCaptured;

}


void Receiver::resume(){

	dataCaptured = false;
	nHeaderBits = nBits = 0;
	stage = start;

	TCCR1B |= _BV(ICES1);
	TIFR |= _BV(ICF1);

}

void Receiver::bitReverse( HCS200data *buffer ){

	uint8_t *data = (uint8_t*) buffer;


	for ( int8_t _byte = 0; _byte < 8 ; _byte++ ){

			uint8_t tempByte = 0;

			for (int8_t _bit = 0; _bit < 7; _bit++){

				tempByte |= !!(data[_byte] & _BV(_bit));
				tempByte <<= 1;

			}

			tempByte |= !!(data[_byte] & _BV(7));
			data[_byte] = tempByte;

	}

}

uint32_t Receiver::getSerialNumber( HCS200data *buffer ){

	uint32_t *data = (uint32_t*) buffer;
	data++;

	return  *data & 0x0FFFFFFF;

}

HCS200data * Receiver::decode( HCS200data *buffer ){

	uint8_t  *data = (uint8_t*)  buffer;
	uint32_t *code = (uint32_t*) buffer; // as data is stored in Little Endian

	//         89F4C76A    BD75C14F
	Keeloq k(0x89F4C76A, 0xBD75C14F);
	//Keeloq k(0x01020304, 0x05060708);
	uint32_t dec = k.decrypt(*code);

	code++;

	// prevent of loosing data
	uint32_t serial_number = *code & 0x0FFFFFFF;
	uint8_t buttons = (*(data + 7) & 0xF0) >> 4;
	bool battery_low = *(data + 8) & 2;
	bool RPT = *(data + 8) & 1;

	buffer->serial_number = serial_number;
	buffer->sync = dec;
	buffer->discrimination_bits = (dec >> 16) & 0x0FFF;
	buffer->buttons = buttons;
	buffer->RPT = RPT;
	buffer->battery_low = battery_low;

	return buffer;

}
