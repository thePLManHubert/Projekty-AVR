#include "Database.h"
#include "utility.h"
#include <avr/io.h>


////////////////////////////////    REMOTE    ////////////////////////////////
Remote::Remote(){
	dbID = 0;
	serial = 0;
	sync = 0;
}

Remote::Remote(uint8_t id){
	dbID = id;
	UTIL::eepromReadBytes(dbID*4 + 1, (uint8_t*)&serial, 2);
	UTIL::eepromReadBytes(dbID*4 + 3, (uint8_t*)&sync,   2);
}

void Remote::updateSync(uint16_t newSync){
	sync = newSync;
	UTIL::eepromWriteBytes(dbID*4 + 3, (uint8_t*)&sync,   2);
}

////////////////////////////////    DATABASE    ////////////////////////////////

Database::Database(){

	remotesCount = UTIL::eepromReadByte(0);

	for(uint8_t i = 0; i < remotesCount; i++){
		remotes[i] = Remote(i); // id pilotów zaczynaj¹ siê od 0
	}

}

bool Database::hasRemote(uint16_t serial){

	bool remoteExist = false;

	for(uint8_t i = 0; i < remotesCount; i++){
		if(remotes[i].serial == serial)
			remoteExist = true;
	}

	return remoteExist;
}

uint8_t Database::getRemoteID(uint16_t serial){

	int idx = remotesCount;

	for(uint8_t i = 0; i < remotesCount; i++){
		if(remotes[i].serial == serial)
			idx = i;
	}

	return idx;
}

Remote * Database::operator[](uint16_t serial){
	return &remotes[getRemoteID(serial)];
}

