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

void LoRaPacket::setSrcId(const char* id) {
	memcpy(getDataAtSrc(), id, srcSize);
}

// String LoRaPacket::getSrcId() {
// 	return String(data).substring(idSize, idSize + srcSize);
// }
const char* LoRaPacket::getSrcId() {
    static char srcId[srcSize + 1];         // Buffer for srcSize characters + null terminator
    
    strncpy(srcId, data + idSize, srcSize); // Copy srcSize characters starting from idSize
    srcId[srcSize] = '\0';

    return srcId;
}

void LoRaPacket::setPacketId(String id) {
	memcpy(getDataAtId(), (char*) id.c_str(), idSize);
}

const char* LoRaPacket::getPacketId() {
    static char packetId[idSize + 1];       // Buffer for idSize characters + null terminator

    for (uint i = 0; i < idSize && data[i] != '\0'; ++i) {
        packetId[i] = data[i];
    }
    packetId[idSize] = '\0';

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

const char* LoRaPacket::getEncryptedData() {
    static char encryptedData[packetSize + idSize]; // Adjust size as needed
    char subPacket[packetSize]; // Temporary buffer for the substring

    // Copying the relevant part of data into subPacket
    strncpy(subPacket, data + idSize, packetSize - idSize);
    subPacket[packetSize - idSize] = '\0'; // Ensure null termination

    // Encrypting the subPacket
    const char* encryptedSubPacket = Encryption::encrypt(subPacket, getPacketId());

    // Concatenating getPacketId() and encryptedSubPacket
    strcpy(encryptedData, getPacketId());
    strcat(encryptedData, encryptedSubPacket);

    return encryptedData;
}

void LoRaPacket::decrypt() {
	String decryptedData = getEncryptedData();
	memcpy(getData(), (char*) decryptedData.c_str(), decryptedData.length());
}

LoRaPacket::~LoRaPacket() {
    delete[] data;
}
