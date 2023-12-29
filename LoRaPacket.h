#ifndef LORAPACKET_H
#define LORAPACKET_H

#include <Arduino.h>
#include <Lora.h>
#include "encryption.h"

#define SEEN_HISTORY 16
#define PACKET_HOPS 8

class LoRaPacket {
public:
	static const size_t idSize = 8;
	static const size_t srcSize = 8;	
	static const size_t hopSize = 3;
	static const size_t modeSize = 1;
	static const size_t messageSize = 254 - 8 - 8 - 3 - 1;
	static const size_t packetSize = 256;

	static const char modeMSG = '0';
	static const char modeACK = '1';

	LoRaPacket();
	LoRaPacket(const char* sourcePacket);
	
	char* getData();

	char* getDataAtId();
	char* getDataAtSrc();
	char* getDataAtHop();
	char* getDataAtMode();
	char* getDataAtMessage();

	void setMessage(const char* message);
	void setSrcId(const char* id);
	const char* getSrcId();
	void setPacketId(const char* id);
	const char* getPacketId();
	bool isNew();
	void setNew();
	bool isRelay();
	void setRelay();
	void setMode(char mode);
	char getMode();
	const char* getEncryptedData();
	void decrypt();

	~LoRaPacket();

private:
	char data[packetSize];
	bool newPacket = false;
	bool relay = false;
};

#endif // LORAPACKET_H