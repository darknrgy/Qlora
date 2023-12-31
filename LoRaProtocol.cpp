#include "LoRaProtocol.h"


// Public LoRaProtocol

LoRaProtocol::LoRaProtocol(LoRaClass* lora) {
	this->lora = lora;
	for (size_t i = 0; i < ACK_QUEUE_SIZE; i++) {
		ackQueue[i][0] = '\0';
	}
}

bool LoRaProtocol::listenAndRelay() {
    bool active = false;
    
    lastReply[0] = '\0'; // Assuming lastReply is a char array
    LoRaPacket* packet = new LoRaPacket();

    receive(packet);

    if (packet->isNew()) {
    	// Copy data from packet to lastReply
        strncpy(lastReply, packet->getDataAtMessage(), LoRaPacket::packetSize - 1);
        lastReply[LoRaPacket::packetSize - 1] = '\0'; // Ensure null termination

        int rssi = LoRa.packetRssi();
        SERIAL_DEBUG_FORMAT(64, "RSSI %d", rssi);

        serialPrint(">>> ");
        serialPrintln(lastReply);
        active = true;
    }

    if (packet->isRelay()) {
        relay(packet);
        active = true;
    }

    delete packet;
    return active;
}

const char* LoRaProtocol::getLastReply() {
	return lastReply;
}

bool LoRaProtocol::receive(LoRaPacket* packet) {
	int packetSize = this->lora->parsePacket();
	if (!packetSize) return false;
	
	long i = 0;
	while (this->lora->available() && i < LoRaPacket::packetSize - 1) {
		packet->getData()[i++] = this->lora->read();
    }
  	packet->getData()[i] = '\0';

	packet->decrypt();
	processReceived(packet);
	return true;
}

// Disable warning about strncpy using a calculated size, we are deliberately comparing
//  the calculated size against the buffer size and taking the smaller.
#pragma GCC diagnostic ignored "-Wstringop-overflow"

void LoRaProtocol::send(const char* message, uint hops) {
	LoRaPacket* packet = &sendQueue[currentPacket];
	inventPacketId(packet);
	packet->reset();
	packet->setMode(LoRaPacket::modeMSG);
	packet->setSrcId(getDeviceId());
	setHopCount(packet, hops);
	packet->setMessage(message);
	currentPacket = (currentPacket + 1) % PACKET_QUEUE_SIZE;
}

void LoRaProtocol::sendNextPacketInQueue() {
	LoRaPacket* packet;
	for (size_t i = 0; i < PACKET_QUEUE_SIZE; i++) {
		packet = &sendQueue[(i + currentPacket) % PACKET_QUEUE_SIZE];
		if (packet->shouldRetry()) {
			loraSend(packet);
			packet->updateNextRetryTime();

			// Always only send one packet, return to main loop for a chance to read
			return;
		}
	}
}

void LoRaProtocol::relay(LoRaPacket* packet) {
	if (!CONFIG.isRelay()) return;
	SERIAL_DEBUG_FORMAT(512, "RELAY: %s", packet->getData());
			
	LoRaPacket* relayPacket = &sendQueue[currentPacket];
	strncpy(relayPacket->getData(), packet->getData(), LoRaPacket::packetSize-1);
	relayPacket->setSrcId(getDeviceId());
	relayPacket->reset();
	addFromMe(relayPacket);

	currentPacket = (currentPacket + 1) % PACKET_QUEUE_SIZE;
}

// Private LoRaProtocol
void LoRaProtocol::sendAckPacket(LoRaPacket* packet) {
	delay(10);
	char srcIdBuff[LoRaPacket::srcSize + 1];
	char packetIdBuff[LoRaPacket::idSize + 1];

	LoRaPacket* ackPacket = new LoRaPacket();
	ackPacket->setSrcId(packet->getSrcId(srcIdBuff));
	ackPacket->setMode(LoRaPacket::modeACK);
	ackPacket->setPacketId(packet->getPacketId(packetIdBuff));
	setHopCount(ackPacket, 0);
	ackPacket->setMessage("ACK");

	loraSend(ackPacket);
	delete ackPacket;
}


void LoRaProtocol::processReceived(LoRaPacket* packet) {
    char packetIdBuff[LoRaPacket::idSize + 1];
    if (isIgnoredSender(packet->getSrcId(packetIdBuff))) return;

    if (packet->getMode() == LoRaPacket::modeACK) {
		for (size_t i = 0; i < PACKET_QUEUE_SIZE; i++) {
			if (!sendQueue[i].isAcked() && strcmp(packet->getPacketId(packetIdBuff), sendQueue[i].getPacketId(packetIdBuff)) == 0) {
				sendQueue[i].setAcked();
				SERIAL_DEBUG_FORMAT(128, "ACKED: %s", packet->getPacketId(packetIdBuff));
				return;
			}
		}
		return;
    }

    if (!isFromMe(packet)) sendAckPacket(packet);
    if (isSeen(packet)) return;

    int hopCount = decrementHopCount(packet);
    if (hopCount >= 1) packet->setRelay();
    if (hopCount >= 0) packet->setNew();
}

bool LoRaProtocol::isSeen(LoRaPacket* packet) {
	char id[LoRaPacket::idSize + 1];
	strncpy(id, packet->getDataAtId(), LoRaPacket::idSize);
	id[LoRaPacket::idSize] = '\0';

	for (int i = 0; i < SEEN_HISTORY; i++) {
		if (strcmp(id, seen[i]) == 0) {
			return true;
		}
	}
	addSeen(id);	
	return false;
}

bool LoRaProtocol::isFromMe(LoRaPacket* packet) {
	char id[LoRaPacket::idSize + 1];
	strncpy(id, packet->getDataAtId(), LoRaPacket::idSize);
	id[LoRaPacket::idSize] = '\0';

	for (int i = 0; i < SEEN_HISTORY; i++) {
		if (strcmp(id, fromMe[i]) == 0) {
			return true;
		}
	}
	return false;
}

uint64_t LoRaProtocol::decrementHopCount(LoRaPacket* packet) {
    char hopStr[LoRaPacket::hopSize + 1];
    strncpy(hopStr, packet->getDataAtHop(), LoRaPacket::hopSize);
    hopStr[LoRaPacket::hopSize] = '\0';

    int hopCount = atoi(hopStr);
    hopCount = hopCount > 0 ? hopCount - 1 : 0;

    setHopCount(packet, hopCount);
    return hopCount;
}

void LoRaProtocol::setHopCount(LoRaPacket* packet, uint64_t hopCount) {
    char hopStr[LoRaPacket::hopSize + 1];
    snprintf(hopStr, LoRaPacket::hopSize + 1, "%0*llu", LoRaPacket::hopSize, hopCount);
    memcpy(packet->getDataAtHop(), hopStr, LoRaPacket::hopSize);
}

// Generate random packet id of 12 hex characters
void LoRaProtocol::inventPacketId(LoRaPacket* packet) {
	static char result[LoRaPacket::idSize + 1];
	unsigned long randomNumber = random(0, 0x7FFFFFFF);
	sprintf(result, "%08lX", randomNumber);
	packet->setPacketId(result);
	addSeen(result);
	addFromMe(packet);
}

void LoRaProtocol::setMessage(LoRaPacket* packet, const char* message){
	packet->setMessage(message);
}

unsigned long LoRaProtocol::loraSend(LoRaPacket* packet) {
	char encryptedDataBuff[LoRaPacket::packetSize + LoRaPacket::idSize];

	const char* original = packet->getData();
	const char* data = packet->getEncryptedData(encryptedDataBuff);
	
	unsigned long long expire;
	unsigned long long dt = 0;
	unsigned long long rxWait = 0;
	unsigned long long rxWaitRandom = 0;

	static char isRxMode;	
	
	SERIAL_DEBUG_FORMAT(512, "SEND: %s", original);

	// wait if the module is has detected an RX signal (clears when RX is done)
	rxWait = 0;
	rxWaitRandom = random(100,500);
	while (true) {
		isRxMode = isInRXMode();
		if (rxWait == 0 && !isRxMode) {
			rxWait = ullmillis() + rxWaitRandom;
		}

		if (isRxMode) {
			// Serial.print(".");
			rxWait = 0;
		}

		if (rxWait && ullmillis() > rxWait) {
			break;
		}
	}

	dt = ullmillis();
	LoRa.beginPacket();
	LoRa.print(data);
	LoRa.endPacket();
	
	dt = ullmillis() - dt;
	
	return dt;
}


void LoRaProtocol::addSeen(const char* id) {
	strncpy(seen[currentSeen], id, LoRaPacket::idSize + 1); 
	currentSeen++;
	if (currentSeen >= SEEN_HISTORY) currentSeen = 0;
}

void LoRaProtocol::addFromMe(LoRaPacket* packet) {
	strncpy(fromMe[currentSeen], packet->getDataAtId(), LoRaPacket::idSize + 1); 
	fromMe[currentSeen][LoRaPacket::idSize] = '\0';
	currentFromMe++;
	if (currentFromMe >= SEEN_HISTORY) currentFromMe = 0;
}

bool LoRaProtocol::isIgnoredSender(const char* sender) {
    if (sender == nullptr || sender[0] == '\0') return false; // Check if sender is empty

    const char* ignoreList = CONFIG.getIgnore();
    if (ignoreList == nullptr) return false;

    // Check if sender is in the ignore list
    const char* found = strstr(ignoreList, sender);
    if (found == nullptr) return false;

    return true;
}


void LoRaProtocol::configure() {
	this->lora->setSpreadingFactor(12);
	this->lora->setSignalBandwidth(CONFIG.getBandwidth());
	this->lora->setCodingRate4(8);
	this->lora->setTxPower(CONFIG.getPower());
	this->lora->setFrequency(CONFIG.getFrequency());
	this->lora->enableLowDataRateOptimize();
}

bool LoRaProtocol::isInRXMode() {
	return LoRa.readRegister(0x18) & 0x03;
}
