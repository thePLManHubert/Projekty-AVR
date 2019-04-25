#ifndef DATABASE_H
#define DATABASE_H
#include <avr/io.h>

class Remote{

public:
	uint8_t dbID;

	uint16_t serial;
	uint16_t sync;

public:
	Remote();
	Remote(uint8_t id);
	void updateSync(uint16_t newSync);

};

class Database {

private:
	uint8_t remotesCount;
	Remote remotes[4];

public:
	Database();
	bool hasRemote(uint16_t serial);
	Remote * operator[](uint16_t serial);

private:
	uint8_t getRemoteID(uint16_t serial);

};



#endif
