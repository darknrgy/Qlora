#ifndef LORAPACKET_H
#define LORAPACKET_H

#include <Arduino.h>
#include <Lora.h>
#include "encryption.h"

#define SEEN_HISTORY 16
#define PACKET_HOPS 8

class LoRaPacket {
public:
	static const uint idSize = 8;
	static const uint srcSize = 8;	
	static const uint hopSize = 3;
	static const uint modeSize = 1;
	static const uint messageSize = 254 - 8- 8 - 3 - 1;
	static const uint64_t packetSize = 256;

	static const char modeMSG = '0';
	static const char modeACK = '1';

	LoRaPacket();
	LoRaPacket(char* sourcePacket);
	
	char* getData();
	void setData(String data);

	char* getDataAtId();
	char* getDataAtSrc();
	char* getDataAtHop();
	char* getDataAtMode();
	char* getDataAtMessage();

	void setMessage(char* message);
	void setSrcId(const char* id);
	const char* getSrcId();
	void setPacketId(String id);
	String getPacketId();
	bool isNew();
	void setNew();
	bool isRelay();
	void setRelay();
	void setMode(char mode);
	char getMode();
	String getEncryptedData();
	void decrypt();

	~LoRaPacket();

private:
	char *data;
	bool newPacket = false;
	bool relay = false;
};

#endif // LORAPACKET_H