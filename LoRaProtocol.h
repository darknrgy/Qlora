#ifndef LORAPROTOCOL_H
#define LORAPROTOCOL_H

#include <Arduino.h>
#include <Lora.h>
#include "LoRaPacket.h"
#include "ullmillis.h"
#include "config.h"
#include "util.h"
#include "encryption.h"

#define SEEN_HISTORY 16
#define PACKET_HOPS 8

class LoRaProtocol {
public:
	LoRaClass* lora;

	LoRaProtocol(LoRaClass* lora);
	bool listenAndRelay();
	const char* getLastReply();
	void receive(LoRaPacket* packet);
	void send(const char* message, uint hops);
	void relay(LoRaPacket* packet);
	void configure();
	bool isIgnoredSender(const char* sender);
	

private:
	char seen[SEEN_HISTORY][LoRaPacket::idSize + 1] = {""};
	char fromMe[SEEN_HISTORY][LoRaPacket::idSize + 1] = {""};

	int currentSeen = 0;
	int currentFromMe = 0;
	int currentIgnoredSender = 0;

	char packetId     [LoRaPacket::idSize + 1];
	char sentPacketId [LoRaPacket::idSize + 1];
	char lastReply    [LoRaPacket::packetSize];

	long nextTxTime = ullmillis();

	void sendAckPacket(LoRaPacket* packet);
	void processReceived(LoRaPacket* packet);
	bool isSeen(LoRaPacket* packet);
	bool isFromMe(LoRaPacket* packet);
	uint64_t decrementHopCount(LoRaPacket* packet);
	void setHopCount(LoRaPacket* packet, uint64_t hopCount);
    void inventPacketId(LoRaPacket* packet);
    void setMessage(LoRaPacket* packet, const char* message);
    unsigned long  loraSend(LoRaPacket* packet);
    void addSeen(const char* id);
    void addFromMe(LoRaPacket* packet);
};

#endif // LORAPROTOCOL_H