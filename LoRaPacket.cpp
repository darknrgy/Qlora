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
	return data + srcSize + idSize + hopSize + modeSize;
}

void LoRaPacket::setMessage(char* message) {
	memcpy(data + srcSize + idSize + hopSize + modeSize, message, messageSize);
	data[255] = '\0';
}

void LoRaPacket::setSrcId(String id) {
	memcpy(data, (char*) id.c_str(), srcSize);
}

String LoRaPacket::getSrcId() {
	return String(data).substring(0, 8);
}

void LoRaPacket::setPacketId(String id) {
	memcpy(data + srcSize, (char*) id.c_str(), idSize);
}

String LoRaPacket::getPacketId() {
	String packetId;
	for (unsigned int i = 0; i < idSize && data[i] != '\0'; ++i) {
		packetId += data[i + srcSize];
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
	data[srcSize + idSize + hopSize] = mode;
}

char LoRaPacket::getMode() {
	return data[srcSize + idSize + hopSize];
}

LoRaPacket::~LoRaPacket() {
    delete[] data;
}
