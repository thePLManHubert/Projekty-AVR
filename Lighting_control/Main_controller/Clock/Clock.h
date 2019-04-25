#ifndef _CLOCK_H
#define _CLOCK_H

#include <avr/io.h>

class Clock {
public:
	static void initClocks(uint16_t globalTime);
	static void setGlobalTime(uint16_t globalTime);
	static uint16_t getGlobalTime();
	static void addMinute();

	Clock();
	Clock( uint8_t hours, uint8_t minutes );
	Clock( uint16_t * time );
	Clock( uint16_t time );

	void start();
	void stop();
	void reset();
	void set( uint8_t hours, uint8_t minutes );
	void set( uint16_t * time );
	void set( uint16_t time );
	uint16_t getTime();
	uint8_t getHours();
	uint8_t getMinutes();
	uint8_t getSeconds();
	void count();

	volatile uint8_t isWorking;

private:
	static volatile uint16_t time;
	uint16_t * timePtr;
	volatile uint16_t insideTime; // minuty i godziny
	volatile uint8_t seconds;

};

#endif
