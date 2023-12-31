#include "LoRaPacket.h"


LoRaPacket::LoRaPacket() {
}

LoRaPacket::LoRaPacket(const char* input) {
	strncpy(data, input, packetSize);
	data[packetSize-1] = '\0';
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

void LoRaPacket::setMessage(const char* message) {
	strncpy(data + idSize + srcSize + hopSize + modeSize, message, messageSize);
	data[packetSize-1] = '\0';
}

void LoRaPacket::setSrcId(const char* id) {
	memcpy(getDataAtSrc(), id, srcSize);
}

const char* LoRaPacket::getSrcId(char* srcIdBuff) {
    srcIdBuff[0] = '\0';
    
    strncpy(srcIdBuff, data + idSize, srcSize); // Copy srcSize characters starting from idSize
    srcIdBuff[srcSize] = '\0';

    return srcIdBuff;
}

void LoRaPacket::setPacketId(const char* id) {
	memcpy(getDataAtId(), id, idSize);
}

const char* LoRaPacket::getPacketId(char* packetId) {
    packetId[0] = '\0';
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

void LoRaPacket::reset() {
	retries = PACKET_RETRIES;
	nextRetryTime = ullmillis();
	acked = false;
}

bool LoRaPacket::shouldRetry() {
	if (retries <= 0) return false;
	if (ullmillis() < nextRetryTime) return false;
	retries--;
	return true;
}

void LoRaPacket::updateNextRetryTime() {
	nextRetryTime = ullmillis() + PACKET_RETRY_TIME;
}

void LoRaPacket::setAcked() {
	acked = true;
	retries = 0;
}

bool LoRaPacket::isAcked() {
	return acked;
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

const char* LoRaPacket::getEncryptedData(char* encryptedDataBuff) {
    char subPacket[packetSize]; // Temporary buffer for the substring

    // Copying the relevant part of data into subPacket
    strncpy(subPacket, data + idSize, packetSize - idSize);
    subPacket[packetSize - idSize] = '\0'; // Ensure null termination

    // Encrypting the subPacket
    char packetIdBuff[LoRaPacket::idSize + 1];
    const char* encryptedSubPacket = Encryption::encrypt(subPacket, getPacketId(packetIdBuff));

    // Concatenating getPacketId() and encryptedSubPacket
    strcpy(encryptedDataBuff, getPacketId(packetIdBuff));
    strcat(encryptedDataBuff, encryptedSubPacket);

    return encryptedDataBuff;
}

void LoRaPacket::decrypt() {
	char encryptedDataBuff[packetSize + idSize];
	strcpy(getData(), getEncryptedData(encryptedDataBuff));
}

LoRaPacket::~LoRaPacket() {}
