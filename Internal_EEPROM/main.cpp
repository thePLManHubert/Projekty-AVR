/* Testowanie zapisu i odczytu wewnêtrznej pamiêci EEPROM mikrokontrolera */

#include <avr/io.h>
#include <util/delay.h>

uint8_t eepromReadByte(uint16_t address);
void eepromWriteByte(uint16_t address, uint8_t data);
void eepromReadBytes(uint16_t address, uint8_t data[], uint16_t size);
void eepromWriteBytes(uint16_t address, uint8_t data[], uint16_t size);


int main( void ){

	DDRA = 0xFF;

/*
	uint32_t val = 0x13F0DCA1;
	uint8_t *ptr = (uint8_t*)&val;

	eepromWriteBytes(0, ptr, 4);
*/
	while(1){

		for(int i = 0; i < 9; i++){
			PORTA = eepromReadByte(i) >> 4;
			_delay_ms(2000);
			PORTA = eepromReadByte(i);
			_delay_ms(2000);
		}

	}

}



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
