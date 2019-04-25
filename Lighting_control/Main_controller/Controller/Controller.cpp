/* Obiekt kontrolera */

#include "Controller.h"

uint8_t Controller::getAddress() {
	return memoryPart.address;
}

uint8_t Controller::getState() {
	uint8_t state = !!(memoryPart.flags & 0b00000001);
	return state;
}

uint8_t Controller::getWorkingHours() {
	uint8_t wh = !!(memoryPart.flags & 0b00000010);
	return wh;
}

uint8_t Controller::getCheckFlag(){
	uint8_t check = !!(memoryPart.flags & 0b00000100);
	return check;
}

void Controller::setCheckFlag(uint8_t b){
	if(b)
		memoryPart.flags |= 0b00000100;
	else
		memoryPart.flags &= ~0b00000100;
}

uint8_t Controller::getErrorFlag(){
	uint8_t err = !!(memoryPart.flags & 0b00001000);
	return err;
}

void Controller::setErrorFlag(uint8_t b){
	if(b)
			memoryPart.flags |= 0b00001000;
		else
			memoryPart.flags &= ~0b00001000;
}

uint16_t Controller::getStartTime() {
	return memoryPart.startTime;
}

uint16_t Controller::getStopTime() {
	return memoryPart.stopTime;
}

uint16_t Controller::getCurrentTimeOn() {
	return currentOnTime;
}

void Controller::zero() {
	memoryPart.address = 0;
	memoryPart.flags = 0;
	memoryPart.startTime = 0;
	memoryPart.stopTime = 0;
}

void Controller::setCurrentOnTime( uint16_t t ) {
	currentOnTime = t;
}

void Controller::setState(uint8_t s){
	if(s == 0)
		memoryPart.flags &= ~_BV(0);
	else
		memoryPart.flags |= _BV(0);
}

void Controller::setMemoryPart( EepromController ec ) {
	memoryPart.address = ec.address;
	memoryPart.flags = ec.flags;
	memoryPart.startTime = ec.startTime;
	memoryPart.stopTime = ec.stopTime;
}
