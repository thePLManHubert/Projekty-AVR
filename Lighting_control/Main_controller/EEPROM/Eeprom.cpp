/* Zapisywanie do pamiêci EEPROM danych o kontrolerach pod³¹czonych do g³ównego mikrokontrolera */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Eeprom.h"
#include "../utilities/Utilities.h"

Eeprom::Eeprom() {
	// ustaw kasowanie i zapis do komórki pamiêci jako operacjê atomow¹
	EECR &= ~(1 << EEPM1) & ~(1 << EEPM0);

	//clear();
	init();
}

uint8_t Eeprom::getNumberOfControllers() {
	return numberOfControllers;
}

bool Eeprom::addNewController( EepromController ec ) {
	// je¿eli w pamiêci znajduje siê kontroler o danym adresie to nie przeprowadzaj operacji
	for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
		if (addresses[i].controllerAddress == ec.address)
			return false;
	}

	uint8_t bytes[EEPROM_CONTROLLER_SIZE];
	Location a = getFirstAvailable();

	if (a.EepromControllerAddress != 0) {
		arrangeControllerBytes( ec, bytes );
		writeBytes( a.EepromControllerAddress, bytes, EEPROM_CONTROLLER_SIZE );
		numberOfControllers++;
		writeByte( 0, numberOfControllers );
		a.controllerAddress = ec.address;
		addresses[a.index] = a;

//		lcd.clear();
//		lcd.goToXY( 0, 1 );
//		lcd.print( a.controllerAddress );
//		lcd.print( " " );
//		lcd.print( a.EepromControllerAddress );
//		lcd.print( " " );
//		lcd.print( a.index );
//		_delay_ms( 1000 );

		return true;
	} else return false;
}

bool Eeprom::deleteController( EepromController ec ) {
	return deleteController( ec.address );
}

bool Eeprom::deleteController( uint8_t address ) {
	uint8_t bytes[EEPROM_CONTROLLER_SIZE] = { 0, 0, 0, 0, 0 };

	for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
		if (addresses[i].controllerAddress == address) {
			writeBytes( addresses[i].EepromControllerAddress, bytes, EEPROM_CONTROLLER_SIZE );
			numberOfControllers--;
			writeByte( 0, numberOfControllers );
			addresses[i].controllerAddress = 0;
			return true;
		}
	}
	return false;
}

bool Eeprom::updateControler( EepromController ec ) {
	uint8_t bytes[EEPROM_CONTROLLER_SIZE];
	arrangeControllerBytes( ec, bytes );

	for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
		if (addresses[i].controllerAddress == ec.address) {
			writeBytes( addresses[i].EepromControllerAddress, bytes, EEPROM_CONTROLLER_SIZE );
			return true;
		}
	}
	return false;
}

bool Eeprom::hasNext() {
	return iterator < numberOfControllers;
}

EepromController Eeprom::getNext() {
	EepromController e = { 0, 0, 0, 0 };

	while (addresses[current].controllerAddress == 0 && current < MAX_CONTROLLER_NUMBER) {
		current++;
	}
	if (hasNext()) {
		e = getController( addresses[current].controllerAddress );
		iterator++;
		current++;
	}

	return e;
}

void Eeprom::resetIterator() {
	current = 0;
	iterator = 0;
}

EepromController Eeprom::getController( uint8_t address ) {
	EepromController controller = { 0, 0, 0, 0 };
	uint8_t bytes[EEPROM_CONTROLLER_SIZE];

	for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
		if (address == addresses[i].controllerAddress) {
			readBytes( addresses[i].EepromControllerAddress, bytes, EEPROM_CONTROLLER_SIZE );
			controller = retrieveFromBytes( bytes );
			break;
		}
	}
	return controller;
}

/*-----------------------------------------------------------------------------------------------*/

void Eeprom::init() {
	// wczytanie liczby kontrolerów z pamiêci EEPROM
	numberOfControllers = readByte( 0 );

	// inicjalizacja tablicy asocjacyjnej (Placement)
	for (int address = 1, i = 0; i < MAX_CONTROLLER_NUMBER; address += EEPROM_CONTROLLER_SIZE, i++) {
		addresses[i].index = i;
		addresses[i].EepromControllerAddress = address;
		addresses[i].controllerAddress = readByte( address );
	}
}

void Eeprom::clear() {
	for (int i = 0; i < 1024; i++) {
		writeByte( i, 0 );
	}
}

Location Eeprom::getFirstAvailable() {
	Location available = { 0, 0, 0 };

	for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
		if (addresses[i].controllerAddress == 0) {
			available = addresses[i];
			break;
		}
	}
	return available;
}

void Eeprom::arrangeControllerBytes( EepromController c, uint8_t * bytes ) {
	bytes[0] = c.address;
	bytes[1] = c.flags;
	bytes[2] = ((c.startTime >> 4) & 0b11110000) | (c.stopTime >> 8);
	bytes[3] = c.startTime;
	bytes[4] = c.stopTime;

//	lcd.clear();
//	lcd.print( "Start Time" );
//	lcd.goToXY(0,1);
//	lcd.print(c.startTime);
//	lcd.print( " " );
//	lcd.print(c.startTime >> 8);
//	_delay_ms( 1000 );
//	lcd.clear();
//	lcd.print( "in" );
//	lcd.goToXY(0,1);
//	lcd.print( bytes[0] );
//	lcd.print( " " );
//	lcd.print( bytes[1] );
//	lcd.print( " " );
//	lcd.print( bytes[2] );
//	lcd.print( " " );
//	lcd.print( bytes[3] );
//	lcd.print( " " );
//	lcd.print( bytes[4] );
//	_delay_ms( 1000 );

}

EepromController Eeprom::retrieveFromBytes( uint8_t * bytes ) {
	EepromController controller;

	controller.address = bytes[0];
	controller.flags = bytes[1];
	controller.startTime = (((uint16_t)(bytes[2] & 0b11110000)) << 4) | bytes[3];
	controller.stopTime = (((uint16_t)(bytes[2] & 0b00001111)) << 8) | bytes[4];

//	lcd.clear();
//	lcd.print( "out" );
//	lcd.goToXY(0,1);
//	lcd.print( controller.address );
//	lcd.print( " " );
//	lcd.print( controller.flags );
//	lcd.print( " " );
//	lcd.print( controller.startTime );
//	lcd.print( " " );
//	lcd.print( controller.stopTime );
//	_delay_ms( 1000 );

	return controller;
}

uint8_t Eeprom::readByte( uint16_t address ) {
	//zaczekaj na zakoñczenie poprzedniej operacji
	while (EECR & (1 << EEPE))
		;

	//ustaw adres
	EEAR = address;

	//rozpocznij odczyt
	EECR |= _BV( EERE );

	//zwróæ dan¹
	return EEDR;
}

void Eeprom::readBytes( uint16_t address, uint8_t * buffer, uint8_t length ) {
	for (uint16_t i = 0; i < length; i++)
		buffer[i] = readByte( address + i );
}

void Eeprom::writeByte( uint16_t address, uint8_t data ) {
	//zaczekaj na zakoñczenie poprzedniej operacji
	while (EECR & (1 << EEPE))
		;

	// za³aduj adres i dan¹ do zapisu
	EEAR = address;
	EEDR = data;

	// zapisz wartoœc rejestru statusu
	char cSREG;
	cSREG = SREG;

	// wy³¹cz przerwania
	cli();

	EECR |= (1 << EEMPE); 	// rozpocznij zapisywanie bajtu
	EECR |= (1 << EEPE);	// zapisz bajt
	SREG = cSREG; 			// przywróc poprzedni¹ wartoœc rejestru statusu

	// w³¹cz przerwania
	sei();
}

void Eeprom::writeBytes( uint16_t address, uint8_t * buffer, uint8_t length ) {
	for (uint16_t i = 0; i < length; i++)
		writeByte( address + i, buffer[i] );
}
