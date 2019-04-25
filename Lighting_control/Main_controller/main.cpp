/* Program g³ówny mikrokontrolera steruj¹cego oœwietleniem po magistrali RS485 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "EEPROM/Eeprom.h"
#include "Serial/Serial.h"
#include "Serial/SoftwareUART.h"
#include "Utilities/Utilities.h"
#include "Definitions.h"
#include "Controller/ControllerManager.h"
#include "utilities/Utilities.h"
#include "Clock/Clock.h"

uint8_t bufferInRS232[16];				// bufor odbioru
uint8_t bufferOutRS232[4];				// bufor wyjœciowy
volatile uint8_t bufferIndexRS232 = 0;	// indeks odbieranego aktualnie bajtu
volatile uint8_t packageCompleteRS232;

uint8_t bufferInRS485[10];				// bufor odbioru
uint8_t bufferOutRS485[5];				// bufor wyjœciowy
volatile uint8_t bufferIndexRS485 = 0;

volatile uint8_t secondPassed;
volatile uint8_t seconds;

void onReceivePackageRS485( uint8_t data );
void timeoutRS485();
void processData( uint8_t * data );

volatile uint16_t millis = 0;
volatile uint8_t readyToSend = 0;	// informuje o gotowoœci do wys³ania danych
/*----------------------------------------------------------------------------------*/

Eeprom ememory = Eeprom();

int main( void ) {
	ControllerManager::init( &ememory );
	HardwareSerial::begin( 9600 );
	SoftwareUART::begin( 9600 );
	SoftwareUART::setTimeout( 2000, NULL );
	SoftwareUART::setCallback( &onReceivePackageRS485 );
	Clock::initClocks( 0 );

	RS485_DDR |= _BV( RS485_PIN );

	//przerwania globalne
	sei();

	while (true) {

		if (packageCompleteRS232) {
			for (int i = 0; i < bufferIndexRS232; i += PACKAGE_SIZE_RS232) {
				ControllerManager::processPackageRS232( bufferInRS232 + i );
			}
			packageCompleteRS232 = 0;
			bufferIndexRS232 = 0;
		}
		if (secondPassed) {
			secondPassed = 0;

			lcd.clear();
			char buf[20];
			sprintf( buf, "%02d:%02d:%02d", Clock::getGlobalTime() / 60, Clock::getGlobalTime() % 60, seconds );
			lcd.print( buf );

			for (int i = 0; i < MAX_CONTROLLER_NUMBER; i++) {
				if (ControllerManager::controllers[i].getAddress()) {
					if (ControllerManager::controllers[i].getState() == STATE_ON) {
						ControllerManager::clocks[i].isWorking = 1;
					} else {
						ControllerManager::clocks[i].isWorking = 0;
					}

					if (ControllerManager::controllers[i].getStartTime() == Clock::getGlobalTime() && ControllerManager::controllers[i].getWorkingHours()) {
						if (seconds == i && ControllerManager::controllers[i].getStartTime() != ControllerManager::controllers[i].getStopTime()){
							ControllerManager::setControllerStateRS485( STATE_ON, ControllerManager::controllers[i].getAddress() );
						}
					}

					if (ControllerManager::controllers[i].getStopTime() == Clock::getGlobalTime() && ControllerManager::controllers[i].getWorkingHours()) {
						if (seconds == i){
							ControllerManager::setControllerStateRS485( STATE_OFF, ControllerManager::controllers[i].getAddress() );
						}
					}

					if (ControllerManager::clocks[i].isWorking) {
						ControllerManager::clocks[i].count();
						ControllerManager::controllers[i].setCurrentOnTime( ControllerManager::clocks[i].getTime() );
					}
				}
			}

		}
	}
}

void onReceivePackageRS485( uint8_t data ) {
	bufferInRS485[bufferIndexRS485++] = data;
	if (bufferIndexRS485 == 5) {
		SoftwareUART::stopCounting();

		if (bufferInRS485[0] == MCU_ADDRESS) {
			processData( bufferInRS485 );
		}
	}
}

void timeoutRS485() {
	bufferIndexRS485 = 0;
	lcd.clear();
	lcd.print( "Timeout." );
	SoftwareUART::stopCounting();
}

void processData( uint8_t * data ) {
	switch (data[MESSAGE_TYPE_IDX_RS485]) {
	case TURN_OFF_PACKAGE:
	case TURN_ON_PACKAGE:
	case INFO_PACKAGE:
	case RESET_TIME_PACKAGE:
		ControllerManager::processPackageRS485(data);
		break;
	}
	bufferIndexRS485 = 0;
}

ISR( TIMER2_COMPA_vect ) { // przerwanie co 1ms

	if (SoftwareUART::isCounting)
		SoftwareUART::counter++;
	if (HardwareSerial::isCounting)
		HardwareSerial::counter++;

	if (SoftwareUART::counter >= SoftwareUART::timeOutLength) {
		timeoutRS485();
		SoftwareUART::stopCounting();
	}

	// je¿eli czas pomiêdzy przychodz¹cymi paczkami jest wiêkszy od 5ms
	// to znaczy, ¿e odbiór danych siê zakoñczy³
	if (HardwareSerial::counter == 5) {
		HardwareSerial::stopCounting();
		packageCompleteRS232 = 1;
	}

}

