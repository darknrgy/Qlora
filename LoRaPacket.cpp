#include "LoRaPacket.h"


LoRaPacket::LoRaPacket() {
	data = new char[packetSize]();
}

LoRaPacket::LoRaPacket(char* data) {
	data = new char[packetSize]();
	if (data != nullptr) {
		strncpy(data, data, 256);
		data[256] = '\0';
	}
}

char* LoRaPacket::getData()  {
	return data;
}

char* LoRaPacket::getMessage() {
	return data + idSize + hopSize + modeSize;
}

void LoRaPacket::setMessage(char* message) {
	memcpy(data + idSize + hopSize + modeSize, message, messageSize);
	data[255] = '\0';
}

void LoRaPacket::setPacketId(String id) {
	setPacketId((char*) id.c_str());
}

void LoRaPacket::setPacketId(char* id) {
	memcpy(data, id, idSize);
}

String LoRaPacket::getPacketId() {
	String packetId;
	for (unsigned int i = 0; i < idSize && data[i] != '\0'; ++i) {
		packetId += data[i];
	}
	return packetId;
}

bool LoRaPacket::isNew() {
	return newPacket;
}

void LoRaPacket::setNew() {
	newPacket = true;
}

bool LoRaPacket::isRelay() {
	return relay;
}

void LoRaPacket::setRelay() {
	relay = true;
}

void LoRaPacket::setMode(char mode) {
	data[idSize + hopSize] = mode;
}

char LoRaPacket::getMode() {
	return data[idSize + hopSize];
}

LoRaPacket::~LoRaPacket() {
    delete[] data;
}
