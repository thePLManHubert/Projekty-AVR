#ifndef HCS200_PROGRAMMER
#define HCS200_PROGRAMMER

#define PROGRAM_PORT_DIR 	DDRB
#define PROGRAM_PORT_READ 	PINB
#define PROGRAM_PORT 		PORTB
#define CLK_PIN  			2 //PROGRAM_PORT
#define DATA_PIN 			1 //PROGRAM_PORT


typedef uint8_t bool;
typedef uint16_t* package;
typedef struct {

	uint64_t encryption_key;
	uint16_t sync;
	uint32_t serial_number;
	uint32_t seed;
	uint16_t config;

} data;


package pack( uint16_t *buffer, data *d ); 	 // min 12 words in buffer
//data unpack ( uint16_t *buffer, package p ); // min 10 words in buffer
void load( package p );
bool verify( package p );


#endif
