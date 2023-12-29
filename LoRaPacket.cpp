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

void LoRaPacket::setData(String data) {
	for (int i = 0; i < data.length(); i++) {
		this->data[i] = data[i];
	}
}

char* LoRaPacket::getDataAtId() {
	return data;
}

char* LoRaPacket::getDataAtSrc() {
	return data + idSize;
}

char* LoRaPacket::getDataAtHop() {
	return data + idSize + srcSize;
}

char* LoRaPacket::getDataAtMode() {
	return data + idSize + srcSize + hopSize;
}

char* LoRaPacket::getDataAtMessage() {
	return data + idSize + srcSize + hopSize + modeSize;
}

void LoRaPacket::setMessage(char* message) {
	memcpy(data + idSize + srcSize + hopSize + modeSize, message, messageSize);
	data[255] = '\0';
}

void LoRaPacket::setSrcId(String id) {
	memcpy(getDataAtSrc(), (char*) id.c_str(), srcSize);
}

String LoRaPacket::getSrcId() {
	return String(data).substring(idSize, idSize + srcSize);
}

void LoRaPacket::setPacketId(String id) {
	memcpy(getDataAtId(), (char*) id.c_str(), idSize);
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
	data[idSize + srcSize + hopSize] = mode;
}

char LoRaPacket::getMode() {
	return data[idSize + srcSize + hopSize];
}

String LoRaPacket::getEncryptedData() {
	String dataString = String(data);
	String subPacket = dataString.substring(idSize, dataString.length());
	subPacket = Encryption::encrypt(subPacket, getPacketId());
	return String(getPacketId() + subPacket);
}

void LoRaPacket::decrypt() {
	String decryptedData = getEncryptedData();
	memcpy(getData(), (char*) decryptedData.c_str(), decryptedData.length());
}

LoRaPacket::~LoRaPacket() {
    delete[] data;
}
