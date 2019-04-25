#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H


/*-----LINIA-RS232------------------------------------------------------------------*/

#define MCU_ADDRESS  				0
#define CONTROLLERS_INFO_PACKAGE  	0	// u¿ywany do wysy³ania paczki z proœb¹ o listê kontrolerów
#define ADD_CONTROLLER_PACKAGE  	1	// u¿ywany do wysy³ania paczki z proœb¹ o dodanie nowego kontrolera do listy MCU
#define REMOVE_CONTROLLER_PACKAGE  	2	// u¿ywany do wysy³ania paczki z proœb¹ o usuniêcie kontrolera z MCU
#define SET_TIME_PACKAGE  			3	// u¿ywany do wysy³ania paczki ustawiaj¹cej czas w MCU
#define GET_TIME_PACKAGE  			4   // u¿ywany do wysy³ania zapytania o czas w MCU

#define STATE_PACKAGE  				10	// informuje czy obiekt pod³¹czony do kontrolera jest w³¹czony czy nie
#define START_TIME_PACKAGE  		11	// informuje o paczce z ustawionym czasem w³¹czania
#define STOP_TIME_PACKAGE  			12	// informuje o paczce z ustawionym czasem wy³¹czania
#define TIME_ON_PACKAGE  			13	// informuje o paczce z czasem uruchomienia

#define ERR_NOT_FOUND_PACKAGE  		20	// informuje o tym, ¿e kontroler nie znajduje siê na liœcie MCU
#define ERR_TIMEOUT_PACKAGE  		21	// informuje o tym, ¿e kontroler nie odpowiada
#define ERR_ADD_NEW_PACKAGE  		22	// informuje o tym, ¿e nie mo¿na dodaæ nowego kontrolera do MCU

#define NO_CONTROLLERS_PACKAGE 		30  // informuje o tym, ¿e na liœcie MCU nie ma ¿adnych kontrolerów

#define ADDRESS_IDX_RS232			0
#define PACKAGE_TYPE_IDX_RS232		1
#define TIME_L_IDX_RS232			2
#define TIME_H_IDX_RS232			3


#define PACKAGE_SIZE_RS232	4


/*-----LINIA-RS485------------------------------------------------------------------*/

#define PORT_RX PINC
#define PIN_RX PINC5
#define PORT_TX PORTC
#define PIN_TX PORTC3
#define DDR_TX DDRC

#define RS485_DDR  		DDRC
#define RS485_PORT 		PORTC
#define RS485_PIN  		PORTC4
#define RS485_TRANSMIT  RS485_PORT |= _BV(RS485_PIN)
#define RS485_RECEIVE   RS485_PORT &= ~_BV(RS485_PIN)

#define TURN_OFF_PACKAGE 	'0'
#define TURN_ON_PACKAGE 	'1'
#define INFO_PACKAGE 		'2'
#define RESET_TIME_PACKAGE 	'3'

#define RECEIVER_IDX_RS485 			0
#define SENDER_IDX_RS485 			1
#define MESSAGE_TYPE_IDX_RS485 		2
#define TIME_L_IDX_RS485 			3
#define TIME_H_IDX_RS485 			4

#define STATE_OFF 	0
#define STATE_ON 	1

#define PACKAGE_SIZE_RS485	5


/*----------------------------------------------------------------------------------*/

#endif
