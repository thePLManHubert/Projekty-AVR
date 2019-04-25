#include "Clock.h"
#include <avr/interrupt.h>
#include "../Controller/ControllerManager.h"
#include "../utilities/Utilities.h"

Clock ControllerManager::clocks[MAX_CONTROLLER_NUMBER];

volatile uint16_t Clock::time;

static volatile uint16_t milliseconds;

extern volatile uint8_t seconds;
extern volatile uint8_t secondPassed;

void Clock::initClocks( uint16_t globalTime ) {
	TCCR0A |= _BV(COM0B1) | _BV( WGM01 ); 	// tryb CTC
	TIMSK0 |= _BV( OCIE0B );				// w³¹czenie przerwania
	OCR0A = 249;							// przerwanie co 1 ms
	TCCR0B |= _BV(CS01) | _BV( CS00 );		// w³¹cz timer 64 preskaler

	setGlobalTime( globalTime );
}

void Clock::setGlobalTime( uint16_t globalTime ) {
	time = globalTime;
}

uint16_t Clock::getGlobalTime() {
	return time;
}

void Clock::addMinute() {
	if (++time == 1440)
		time = 0;
}

/*--------------------------------------------------------------*/

Clock::Clock()
		: isWorking( 0 ), timePtr( 0 ), insideTime( 0 ), seconds( 0 ) {
}

Clock::Clock( uint8_t hours, uint8_t minutes ) {
	set( hours, minutes );
	isWorking = 0;
}

Clock::Clock( uint16_t * time ) {
	set( time );
	isWorking = 0;
}

Clock::Clock( uint16_t time ) {
	set( time );
	isWorking = 0;
}

void Clock::start() {
	isWorking = 1;
}

void Clock::stop() {
	isWorking = 0;
}

void Clock::reset() {
	insideTime = 0;
	seconds = 0;
	*timePtr = 0;
}

void Clock::count() {
	if (++seconds == 60) {
		seconds = 0;
		insideTime++;
		if (timePtr)
			*timePtr = insideTime;
		if (insideTime == 1440) {
			insideTime = 0;
			if (timePtr)
				*timePtr = insideTime;
		}
	}
}

void Clock::set( uint8_t hours, uint8_t minutes ) {
	insideTime = hours * 60 + minutes;
	timePtr = 0;
	seconds = 0;
}

void Clock::set( uint16_t * time ) {
	timePtr = time;
	insideTime = *time;
	seconds = 0;
}

void Clock::set( uint16_t time ) {
	insideTime = time;
	timePtr = 0;
	seconds = 0;
}

uint16_t Clock::getTime() {
	return insideTime;
}

uint8_t Clock::getHours() {
	return insideTime / 60;
}

uint8_t Clock::getMinutes() {
	return insideTime % 60;
}

uint8_t Clock::getSeconds() {
	return seconds;
}


ISR(TIMER0_COMPB_vect) {

	if (++milliseconds == 1000) {
		milliseconds = 0;

		secondPassed = 1;

		if (++seconds == 60) {
			seconds = 0;
			Clock::addMinute();
		}
	}

}
