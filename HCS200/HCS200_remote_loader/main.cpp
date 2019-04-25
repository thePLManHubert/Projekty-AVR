/* Program służący do ładowania danych o pilotach RC do pamięci mikrokontrolera */

#include "utility.h"
#include <avr/io.h>


int main( void ){


	uint16_t serial1 = 0x1234;
	uint16_t serial2 = 0x8E1A;
	uint16_t serial3 = 0x3C18;
	uint16_t sync1 = 0;
	uint16_t sync2 = 0;
	uint16_t sync3 = 0;

	UTIL::eepromWriteByte(0, 3);
	UTIL::eepromWriteBytes(1, (uint8_t*)&serial1, 2);
	UTIL::eepromWriteBytes(3, (uint8_t*)&sync1, 2);
	UTIL::eepromWriteBytes(5, (uint8_t*)&serial2, 2);
	UTIL::eepromWriteBytes(7, (uint8_t*)&sync2, 2);
	UTIL::eepromWriteBytes(9, (uint8_t*)&serial3, 2);
	UTIL::eepromWriteBytes(11, (uint8_t*)&sync3, 2);

}
