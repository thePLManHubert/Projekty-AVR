#ifndef _MANAGER_H
#define _MANAGER_H

#include <avr/io.h>
#include "Controller.h"
#include "../Definitions.h"
#include "../Clock/Clock.h"


class ControllerManager {
public:
	static void init( Eeprom * e );
	static uint8_t getNumberOfControllers();
	static Controller * getController(uint8_t address);
	static void processPackageRS232( uint8_t * data );
	static void setControllerStateRS485(uint8_t state, uint8_t address);
	static void processPackageRS485(uint8_t * data);
	static void sendInfoRS485(uint8_t address);

	static Controller controllers[MAX_CONTROLLER_NUMBER];
	static Clock clocks[MAX_CONTROLLER_NUMBER];

private:
	static Eeprom * eeprom;
	static uint8_t numberOfControllers;

	static void processControllerInfoPackage(uint8_t * data);
	static void processAddControlerPackage(uint8_t * data);
	static void processRemoveControllerPackage(uint8_t * data);
	static void processSetTimePackage(uint8_t * data);
	static void processGetTimePackage(uint8_t * data);
	static void processStatePackage(uint8_t * data, uint8_t address);
	static void processStartTimePackage(uint8_t * data, uint8_t address);
	static void processStopTimePackage(uint8_t * data, uint8_t address);
	static void processTimeOnPackage(uint8_t * data, uint8_t address);
	static void processErrNotFoundPackage(uint8_t * data, uint8_t address);
	static void processErrTimeoutPackage(uint8_t * data, uint8_t address);
	static void processErrAddNewPackage(uint8_t * data, uint8_t address);
};

#endif
