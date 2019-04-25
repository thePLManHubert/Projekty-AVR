#ifndef HCS200_DECODER
#define HCS200_DECODER


#define UNIT_LENGTH 400 //us
#define DEVIATION 	200 //us
#define ZERO 		  2
#define ONE  		  1
#define HEADER 		 10
#define RECV_PIN 	  0 // PORTB



typedef enum {

	start,
	preamble,
	header,
	data

} transmission;

typedef struct{

	uint32_t serial_number;
	uint16_t sync;
	uint16_t discrimination_bits;
	uint8_t buttons;
	bool battery_low;
	bool RPT; // should be logic one

} HCS200data;



class Receiver{

private:
	transmission stage;
	int nHeaderBits;
	int nBits;
	bool dataCaptured;

public:
	Receiver();
	bool receive( HCS200data *buffer );
	HCS200data * decode( HCS200data *buffer );
	uint32_t getSerialNumber( HCS200data *buffer );
	void resume();

private:
	void bitReverse( HCS200data *buffer );

};


#endif
