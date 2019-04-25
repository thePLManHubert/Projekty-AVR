#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <avr/io.h>
#include "../EEPROM/Eeprom.h"

class Controller {
public:
	uint8_t getAddress();
	uint8_t getState();
	uint8_t getCheckFlag();
	void setCheckFlag(uint8_t b);
	uint8_t getErrorFlag();
	void setErrorFlag(uint8_t b);
	uint8_t getWorkingHours();
	uint16_t getStartTime();
	uint16_t getStopTime();
	uint16_t getCurrentTimeOn();
	void zero();
	void setState(uint8_t s);
	void setCurrentOnTime(uint16_t t);
	void setMemoryPart(EepromController ec);

private:
	EepromController memoryPart;
	uint16_t currentOnTime;

};







#endif
