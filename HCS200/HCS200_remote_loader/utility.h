#include <avr/io.h>

namespace UTIL{

uint8_t eepromReadByte(uint16_t address) {
	//zaczekaj na zakoñczenie poprzedniej operacji
	while(EECR & _BV(EEPE));
	//ustaw adres
	EEAR = address;
	//rozpocznij odczyt
	EECR |= _BV(EERE);
	//zwróæ dan¹
	return EEDR;
}
void eepromWriteByte(uint16_t address, uint8_t data) {
	//zaczekaj na zakoñczenie poprzedniej operacji
	while(EECR & _BV(EEPE));
	//ustaw adres i dan¹
	EEAR = address;
	EEDR = data;
	//odblokuj zapis
	EECR |= _BV(EEMPE);
	//rozpocznij zapis
	EECR |= _BV(EEPE);
}

void eepromReadBytes(uint16_t address, uint8_t data[], uint16_t size) {

	for (uint16_t i = 0; i < size; i++) {
		data[i] = eepromReadByte(address + i);
	}

}

void eepromWriteBytes(uint16_t address, uint8_t data[], uint16_t size) {

	for (uint16_t i = 0; i < size; i++) {
		eepromWriteByte(address + i, data[i]);
	}

}

}
//end of UTIL
