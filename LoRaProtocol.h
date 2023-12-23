#ifndef LORAPROTOCOL_H
#define LORAPROTOCOL_H

#include <Arduino.h>
#include <Lora.h>
#include "LoRaPacket.h"
#include "ullmillis.h"
#include "config.h"

#define SEEN_HISTORY 16
#define PACKET_HOPS 8

class LoRaProtocol {
public:
	LoRaProtocol(LoRaClass* lora);
	void listenAndRelay();
	void receive(LoRaPacket* packet);
	void send(String message, uint hops);
	void relay(LoRaPacket* packet);
	void configure();
	

private:
	char seen[SEEN_HISTORY][LoRaPacket::idSize + 1] = {""};
	char fromMe[SEEN_HISTORY][LoRaPacket::idSize + 1] = {""};
	int currentSeen = 0;
	int currentFromMe = 0;
	String packetId;
	String sentPacketId = "";

	LoRaClass* lora;
	void sendAckPacket(LoRaPacket* packet);
	void processReceived(LoRaPacket* packet);
	bool isSeen(LoRaPacket* packet);
	bool isFromMe(LoRaPacket* packet);
	uint64_t decrementHopCount(LoRaPacket* packet);
	void setHopCount(LoRaPacket* packet, uint64_t hopCount);
    void inventPacketId(LoRaPacket* packet);
    void setMessage(LoRaPacket* packet, char* message);
    unsigned long  loraSend(LoRaPacket* packet);
    void addSeen(char id[LoRaPacket::idSize + 1]);
    void addFromMe(LoRaPacket* packet);
};

#endif // LORAPROTOCOL_H