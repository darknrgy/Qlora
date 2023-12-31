#ifndef LORAPACKET_H
#define LORAPACKET_H

#include <Arduino.h>
#include <Lora.h>
#include "encryption.h"
#include "ullmillis.h"

#define SEEN_HISTORY 16
#define PACKET_HOPS 8
#define PACKET_RETRIES 3
#define PACKET_RETRY_TIME 3000

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
	const char* getSrcId(char* srcIdBuff);
	void setPacketId(const char* id);
	const char* getPacketId(char* packetId);
	bool isNew();
	void setNew();
	void reset();
	bool shouldRetry();
	void updateNextRetryTime();
	void setAcked();
	bool isAcked();
	bool isRelay();
	void setRelay();
	void setMode(char mode);
	char getMode();
	const char* getEncryptedData(char* encryptedDataBuff);
	void decrypt();

	~LoRaPacket();

private:
	char data[packetSize];
	bool newPacket = false;
	bool relay = false;
	bool acked = true;
	size_t retries = 0;
	uint64_t nextRetryTime = 0;
};

#endif // LORAPACKET_H