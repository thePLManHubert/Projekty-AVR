#ifndef _EEPROM_H
#define _EEPROM_H
#include <avr/io.h>

// liczba bajtów które kontroler zajmuje w pamiêci EEPROM
const uint8_t EEPROM_CONTROLLER_SIZE = 5;

// maksymalna liczba kontrolerów jakie mog¹ zostac po³¹czone z MCU
const uint8_t MAX_CONTROLLER_NUMBER = 15;


// struktura podstawowa kontrolera
typedef struct {
	uint8_t address;
	uint8_t flags;	// (LSB) 0 - flaga stanu elementu pod³¹czonego do kontrolera (w³/wy³)
						// 		 1 - flaga godzin pracy
						// reszta zarezerwowana na przysz³oœc
	uint16_t startTime;
	uint16_t stopTime;
} EepromController;


// struktura wi¹¿¹ca fizyczny adres kontrolera z adresem w pamiêci EEPROM
typedef struct {
	uint8_t index;
	uint8_t controllerAddress;
	uint8_t EepromControllerAddress;
} Location;


class Eeprom {
public:
	Eeprom();
	uint8_t getNumberOfControllers();
	bool addNewController(EepromController ec);
	bool deleteController(EepromController ec);
	bool deleteController(uint8_t address);
	bool updateControler(EepromController ec);
	bool hasNext();
	EepromController getNext();
	void resetIterator();
	EepromController getController(uint8_t address);
	void clear();

private:
	uint8_t current;
	uint8_t iterator;
	uint8_t numberOfControllers;
	Location addresses[MAX_CONTROLLER_NUMBER];

	void init();
	Location getFirstAvailable();

	uint8_t readByte(uint16_t address);
	void readBytes(uint16_t address, uint8_t * buffer, uint8_t length);
	void writeByte(uint16_t address, uint8_t data);
	void writeBytes(uint16_t address, uint8_t * buffer, uint8_t length);
	void arrangeControllerBytes(EepromController c, uint8_t * bytes);
	EepromController retrieveFromBytes(uint8_t * bytes);
};


#endif
