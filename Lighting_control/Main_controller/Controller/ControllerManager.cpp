/* Reagowanie na dane otrzymane z komputera */

#include "ControllerManager.h"
#include "../Serial/Serial.h"
#include "../Serial/SoftwareUART.h"
#include "../utilities/Utilities.h"

extern uint8_t bufferOutRS232[4];
extern uint8_t bufferOutRS485[5];
extern volatile uint8_t bufferIndexRS232;

uint8_t ControllerManager::numberOfControllers;
Controller ControllerManager::controllers[MAX_CONTROLLER_NUMBER];
Eeprom * ControllerManager::eeprom;

void ControllerManager::init( Eeprom * e ) {
	eeprom = e;
	numberOfControllers = eeprom->getNumberOfControllers();

	int i = 0;
	while (eeprom->hasNext()) {
		controllers[i++].setMemoryPart( eeprom->getNext() );
	}
}

uint8_t ControllerManager::getNumberOfControllers() {
	return numberOfControllers;
}

Controller * ControllerManager::getController( uint8_t address ) {
	Controller * ptr = 0;

	for (int i = 0; i < numberOfControllers; i++) {
		if (controllers[i].getAddress() == address)
			ptr = &controllers[i];
	}
	return ptr;
}

void ControllerManager::processPackageRS232( uint8_t * data ) {

// kopiowanie danych do bufora pomocniczego
	if (data[ADDRESS_IDX_RS232] == MCU_ADDRESS) {
		switch (data[PACKAGE_TYPE_IDX_RS232]) {
		case CONTROLLERS_INFO_PACKAGE:
			processControllerInfoPackage( data );
		break;
		case ADD_CONTROLLER_PACKAGE:
			processAddControlerPackage( data );
		break;
		case REMOVE_CONTROLLER_PACKAGE:
			processRemoveControllerPackage( data );
		break;
		case SET_TIME_PACKAGE:
			processSetTimePackage( data );
		break;
		case GET_TIME_PACKAGE:
			processGetTimePackage( data );
		break;
		}
	} else {
		switch (data[PACKAGE_TYPE_IDX_RS232]) {
		case STATE_PACKAGE:
			processStatePackage( data, data[ADDRESS_IDX_RS232] );
		break;
		case START_TIME_PACKAGE:
			processStartTimePackage( data, data[ADDRESS_IDX_RS232] );
		break;
		case STOP_TIME_PACKAGE:
			processStopTimePackage( data, data[ADDRESS_IDX_RS232] );
		break;
		case TIME_ON_PACKAGE:
			processTimeOnPackage( data, data[ADDRESS_IDX_RS232] );
		break;
		case ERR_NOT_FOUND_PACKAGE:
			processErrNotFoundPackage( data, data[ADDRESS_IDX_RS232] );
		break;
		case ERR_TIMEOUT_PACKAGE:
			processErrTimeoutPackage( data, data[ADDRESS_IDX_RS232] );
		break;
		case ERR_ADD_NEW_PACKAGE:
			processErrAddNewPackage( data, data[ADDRESS_IDX_RS232] );
		break;
		}
	}
}

void ControllerManager::processControllerInfoPackage( uint8_t * data ) {
	if (numberOfControllers == 0) {
		bufferOutRS232[ADDRESS_IDX_RS232] = MCU_ADDRESS;
		bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = NO_CONTROLLERS_PACKAGE;
		bufferOutRS232[TIME_L_IDX_RS232] = 0;
		bufferOutRS232[TIME_H_IDX_RS232] = 0;
		HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
		return;
	}

	for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
		if (controllers[i].getAddress() != 0) {
			bufferOutRS232[ADDRESS_IDX_RS232] = controllers[i].getAddress();
			bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = STATE_PACKAGE;
			bufferOutRS232[TIME_L_IDX_RS232] = controllers[i].getState();
			bufferOutRS232[TIME_H_IDX_RS232] = controllers[i].getWorkingHours();
			HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );

			bufferOutRS232[ADDRESS_IDX_RS232] = controllers[i].getAddress();
			bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = START_TIME_PACKAGE;
			bufferOutRS232[TIME_L_IDX_RS232] = controllers[i].getStartTime();
			bufferOutRS232[TIME_H_IDX_RS232] = controllers[i].getStartTime() >> 8;
			HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );

			bufferOutRS232[ADDRESS_IDX_RS232] = controllers[i].getAddress();
			bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = STOP_TIME_PACKAGE;
			bufferOutRS232[TIME_L_IDX_RS232] = controllers[i].getStopTime();
			bufferOutRS232[TIME_H_IDX_RS232] = controllers[i].getStopTime() >> 8;
			HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );

			bufferOutRS232[ADDRESS_IDX_RS232] = controllers[i].getAddress();
			bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = TIME_ON_PACKAGE;
			bufferOutRS232[TIME_L_IDX_RS232] = controllers[i].getCurrentTimeOn();
			bufferOutRS232[TIME_H_IDX_RS232] = controllers[i].getCurrentTimeOn() >> 8;
			HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
		}
	}
}

void ControllerManager::processAddControlerPackage( uint8_t * data ) {
	uint8_t address = data[TIME_L_IDX_RS232];
	EepromController ec = { address, 0, 0, 0 };
	Controller c = Controller();
	c.setMemoryPart( ec );

	bufferOutRS232[ADDRESS_IDX_RS232] = address;
	bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = ADD_CONTROLLER_PACKAGE;

	if (eeprom->addNewController( ec )) {
		for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
			if (controllers[i].getAddress() == 0) {
				controllers[i].setMemoryPart( ec );
				numberOfControllers = eeprom->getNumberOfControllers();
				break;
			}
		}
	} else {
		processErrAddNewPackage( bufferOutRS232, address );
	}
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processRemoveControllerPackage( uint8_t * data ) {
	uint8_t address = data[TIME_L_IDX_RS232];

	bufferOutRS232[ADDRESS_IDX_RS232] = address;
	bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = REMOVE_CONTROLLER_PACKAGE;

	if (eeprom->deleteController( address )) {
		for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
			if (controllers[i].getAddress() == address) {
				controllers[i].zero();
				numberOfControllers = eeprom->getNumberOfControllers();
				break;
			}
		}
	} else {
		processErrNotFoundPackage( bufferOutRS232, address );
	}
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processSetTimePackage( uint8_t * data ) {
	uint16_t t = ((uint16_t)data[TIME_H_IDX_RS232]) << 8 | data[TIME_L_IDX_RS232];
	Clock::setGlobalTime( t );

	bufferOutRS232[ADDRESS_IDX_RS232] = MCU_ADDRESS;
	bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = SET_TIME_PACKAGE;
	bufferOutRS232[TIME_L_IDX_RS232] = Clock::getGlobalTime();
	bufferOutRS232[TIME_H_IDX_RS232] = Clock::getGlobalTime() >> 8;
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processGetTimePackage( uint8_t * data ) {
	bufferOutRS232[ADDRESS_IDX_RS232] = MCU_ADDRESS;
	bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = GET_TIME_PACKAGE;
	bufferOutRS232[TIME_L_IDX_RS232] = Clock::getGlobalTime();
	bufferOutRS232[TIME_H_IDX_RS232] = Clock::getGlobalTime() >> 8;
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processStatePackage( uint8_t * data, uint8_t address ) {
	Controller * c = getController( address );
	bufferOutRS232[ADDRESS_IDX_RS232] = address;

	if (c) {
		uint8_t state = data[TIME_L_IDX_RS232];
		uint8_t wh = data[TIME_H_IDX_RS232];
		uint8_t flags = wh << 1 | state;

		setControllerStateRS485(state, address);

		bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = STATE_PACKAGE;
		bufferOutRS232[TIME_L_IDX_RS232] = c->getState();
		bufferOutRS232[TIME_H_IDX_RS232] = c->getWorkingHours();

		if (wh != c->getWorkingHours()) {
			EepromController ec = { address, flags, c->getStartTime(), c->getStopTime() };
			if (eeprom->updateControler( ec )) {
				c->setMemoryPart(ec);
				bufferOutRS232[TIME_H_IDX_RS232] = c->getWorkingHours();
			} else {
				processErrNotFoundPackage( bufferOutRS232, address );
			}
		}
	} else {
		processErrNotFoundPackage( bufferOutRS232, address );
	}
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processStartTimePackage( uint8_t * data, uint8_t address ) {
	Controller * c = getController( address );
	bufferOutRS232[ADDRESS_IDX_RS232] = address;

	if (c) {
		uint16_t startTime = ((uint16_t)data[TIME_H_IDX_RS232]) << 8 | data[TIME_L_IDX_RS232];
		uint8_t flags = c->getWorkingHours() << 1 | c->getState();

		EepromController ec = { address, flags, startTime, c->getStopTime() };

		if (eeprom->updateControler( ec )) {
			c->setMemoryPart( ec );

			bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = START_TIME_PACKAGE;
			bufferOutRS232[TIME_L_IDX_RS232] = c->getStartTime();
			bufferOutRS232[TIME_H_IDX_RS232] = c->getStartTime() >> 8;
		} else {
			processErrNotFoundPackage( bufferOutRS232, address );
		}
	} else {
		processErrNotFoundPackage( bufferOutRS232, address );
	}
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processStopTimePackage( uint8_t * data, uint8_t address ) {
	Controller * c = getController( address );
	bufferOutRS232[ADDRESS_IDX_RS232] = address;

	if (c) {
		uint16_t stopTime = ((uint16_t)data[TIME_H_IDX_RS232]) << 8 | data[TIME_L_IDX_RS232];
		uint8_t flags = c->getWorkingHours() << 1 | c->getState();

		EepromController ec = { address, flags, c->getStartTime(), stopTime };

		if (eeprom->updateControler( ec )) {
			c->setMemoryPart( ec );

			bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = STOP_TIME_PACKAGE;
			bufferOutRS232[TIME_L_IDX_RS232] = c->getStopTime();
			bufferOutRS232[TIME_H_IDX_RS232] = c->getStopTime() >> 8;
		} else {
			processErrNotFoundPackage( bufferOutRS232, address );
		}
	} else {
		processErrNotFoundPackage( bufferOutRS232, address );
	}
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processTimeOnPackage( uint8_t * data, uint8_t address ) {
	Controller * c = getController( address );
	bufferOutRS232[ADDRESS_IDX_RS232] = address;

	if (c) {
		bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = TIME_ON_PACKAGE;
		bufferOutRS232[TIME_L_IDX_RS232] = c->getCurrentTimeOn();
		bufferOutRS232[TIME_H_IDX_RS232] = c->getCurrentTimeOn() >> 8;
	} else {
		processErrNotFoundPackage( bufferOutRS232, address );
	}
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}

void ControllerManager::processErrNotFoundPackage( uint8_t * data, uint8_t address ) {
	bufferOutRS232[ADDRESS_IDX_RS232] = address;
	bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = ERR_NOT_FOUND_PACKAGE;
}

void ControllerManager::processErrTimeoutPackage( uint8_t * data, uint8_t address ) {
	bufferOutRS232[ADDRESS_IDX_RS232] = address;
	bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = ERR_TIMEOUT_PACKAGE;
}

void ControllerManager::processErrAddNewPackage( uint8_t * data, uint8_t address ) {
	bufferOutRS232[ADDRESS_IDX_RS232] = address;
	bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = ERR_ADD_NEW_PACKAGE;
	// odrzuc resztê danych z bufora
	bufferIndexRS232 = 0;
}

void ControllerManager::setControllerStateRS485(uint8_t state, uint8_t address){
uint8_t messageType = (state) ? TURN_ON_PACKAGE : TURN_OFF_PACKAGE;

		bufferOutRS485[RECEIVER_IDX_RS485] = address;
		bufferOutRS485[SENDER_IDX_RS485] = MCU_ADDRESS;
		bufferOutRS485[MESSAGE_TYPE_IDX_RS485] = messageType;
		bufferOutRS485[TIME_L_IDX_RS485] = 85;
		bufferOutRS485[TIME_H_IDX_RS485] = 85;

		SoftwareUART::sendBytes(bufferOutRS485, PACKAGE_SIZE_RS485);
}

void ControllerManager::sendInfoRS485(uint8_t address){
		bufferOutRS485[RECEIVER_IDX_RS485] = address;
		bufferOutRS485[SENDER_IDX_RS485] = MCU_ADDRESS;
		bufferOutRS485[MESSAGE_TYPE_IDX_RS485] = INFO_PACKAGE;
		bufferOutRS485[TIME_L_IDX_RS485] = 85;
		bufferOutRS485[TIME_H_IDX_RS485] = 85;

		SoftwareUART::sendBytes(bufferOutRS485, PACKAGE_SIZE_RS485);
}

void ControllerManager::processPackageRS485(uint8_t * data){
	lcd.clear();
	char buf[20];
	sprintf( buf, "%02d:%02d:%02d:%02d", data[0], data[1], data[2], data[3] );
	lcd.print( buf );

	uint8_t address = data[SENDER_IDX_RS485];
	Controller * c = getController( address );
	uint8_t state = 0;
	if (data[MESSAGE_TYPE_IDX_RS485] == TURN_ON_PACKAGE)
		state = STATE_ON;
	else if (data[MESSAGE_TYPE_IDX_RS485] == TURN_OFF_PACKAGE)
		state = STATE_OFF;

	if(c && (state + data[SENDER_IDX_RS485] == data[TIME_L_IDX_RS485])){

		c->setState(state);

		bufferOutRS232[ADDRESS_IDX_RS232] = address;
		bufferOutRS232[PACKAGE_TYPE_IDX_RS232] = STATE_PACKAGE;
		bufferOutRS232[TIME_L_IDX_RS232] = c->getState();
		bufferOutRS232[TIME_H_IDX_RS232] = c->getWorkingHours();
	} else {
		processErrNotFoundPackage( bufferOutRS232, address );
	}
	HardwareSerial::sendBytes( bufferOutRS232, PACKAGE_SIZE_RS232 );
}
