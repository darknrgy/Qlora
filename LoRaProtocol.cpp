#include "LoRaProtocol.h"


// Public LoRaProtocol

LoRaProtocol::LoRaProtocol(LoRaClass* lora) {
	sentPacketId[0] = '\0'; // Init to empty string
	this->lora = lora;
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

        SERIAL_DEBUG_FORMAT(64, "RSSI %d ", LoRa.packetRssi());

        Serial.print(">>> ");
        Serial.println(lastReply);
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

void LoRaProtocol::receive(LoRaPacket* packet) {
	int packetSize = this->lora->parsePacket();
	if (!packetSize) return;
	
	long i = 0;
	while (this->lora->available()) {
		packet->getData()[i++] = this->lora->read();
	}

	// packet->setMessage((char*) packet->getEncryptedData().c_str());

	packet->decrypt();
	processReceived(packet);
}

void LoRaProtocol::send(const char* message, uint hops) {
    while (ullmillis() < nextTxTime);

    static const size_t msgLength = LoRaPacket::messageSize;
    size_t start = 0;
    long int dt = 0;
    char subMessage[LoRaPacket::messageSize + 1]; // Buffer for message slice, +1 for null terminator

    while (strlen(message + start) > 0) {
        delay(dt);
        // Calculate length of substring to take
        size_t length = strlen(message + start) > msgLength ? msgLength : strlen(message + start);
        
        // Copy substring into subMessage
        strncpy(subMessage, message + start, length);
        subMessage[length] = '\0'; // Ensure null termination

        // Update start position for next iteration
        start += length;

        LoRaPacket* packet = new LoRaPacket();
        packet->setNew();
        packet->setMode(LoRaPacket::modeMSG);
        packet->setSrcId(getDeviceId());
        inventPacketId(packet);
        setHopCount(packet, hops);
        packet->setMessage(subMessage);
        dt = loraSend(packet);    
    }
}

void LoRaProtocol::relay(LoRaPacket* packet) {
	if (!CONFIG.isRelay()) return;
	SERIAL_DEBUG_FORMAT(512, "RELAY: %s", packet->getData());
	addFromMe(packet);
	packet->setSrcId(getDeviceId());
	loraSend(packet);
}

// Private LoRaProtocol
void LoRaProtocol::sendAckPacket(LoRaPacket* packet) {
	delay(50);
	LoRaPacket* ackPacket = new LoRaPacket();
	ackPacket->setSrcId(packet->getSrcId());
	ackPacket->setMode(LoRaPacket::modeACK);
	ackPacket->setPacketId(packet->getPacketId());
	setHopCount(ackPacket, 0);
	ackPacket->setMessage("ACK");

	loraSend(ackPacket);
	delete ackPacket;
}


void LoRaProtocol::processReceived(LoRaPacket* packet) {
    if (isIgnoredSender(packet->getSrcId())) return;

    if (packet->getMode() == LoRaPacket::modeACK) {
        if (sentPacketId[0] == '\0') return;  // Check if sentPacketId is empty
        if (strcmp(packet->getPacketId(), sentPacketId) != 0) return; // Compare strings

        // Clear sentPacketId since it's been ack'd
        sentPacketId[0] = '\0';
        SERIAL_DEBUG_FORMAT(128, "ACKED: %s", packet->getPacketId());
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
	const char* original = packet->getData();
	const char* data = packet->getEncryptedData();
	LoRaPacket* ackPacket = new LoRaPacket();
	unsigned long long expire;
	unsigned long long dt = 0;
	
	for (int i = 0; i < 3; i++) {
		SERIAL_DEBUG_FORMAT(512, "SEND: %s", original);
		
		dt = ullmillis();
		LoRa.beginPacket();
		LoRa.print(data);
		LoRa.endPacket();
		if (packet->getMode() == LoRaPacket::modeACK) return dt - ullmillis();

		// Now wait for an ACK
		strncpy(sentPacketId, packet->getPacketId(), LoRaPacket::idSize);
		sentPacketId[LoRaPacket::idSize] = '\0';

		expire = ullmillis() + 3000;
		while (ullmillis() < expire) {
			receive(ackPacket);
			
			// sent packet is empty because it got ack'd
			if (sentPacketId[0] == '\0') {
				dt = ullmillis() - dt;
				nextTxTime = ullmillis() + dt;

				SERIAL_DEBUG_FORMAT(64, "DONE: %lums", dt);

				delete ackPacket;
				return dt;
			}
		}
	}

	sentPacketId[0] = '\0';
	delete ackPacket;
	dt = ullmillis() - dt;
	
	SERIAL_LOG_FORMAT(64, "***NO ACK***: %s %lu", packet->getPacketId(), dt);

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

bool LoRaProtocol::isIgnoredSender(String sender) {
	if (sender.isEmpty()) return false;
	String ignore = CONFIG.getIgnore();
	if (ignore.indexOf(sender) == -1) return false;
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


