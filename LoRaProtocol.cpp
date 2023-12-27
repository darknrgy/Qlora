#include "LoRaProtocol.h"

// Public LoRaProtocol

LoRaProtocol::LoRaProtocol(LoRaClass* lora) {
	this->lora = lora;
}

bool LoRaProtocol::listenAndRelay() {
	bool active = false;
	lastReply = "";
	LoRaPacket* packet = new LoRaPacket();
	receive(packet);

	if (packet->isNew()) {
		lastReply = String(packet->getDataAtMessage());
		
		if (CONFIG.isDebug()) {
			Serial.print("RSSI " + String(LoRa.packetRssi()) + " ");
		}

		Serial.println(">>> " + lastReply);
		active = true;
	}

	if (packet->isRelay()) {
		relay(packet);
		active = true;
	}

	delete packet;
	return active;
}

String LoRaProtocol::getLastReply() {
	return lastReply;
}

void LoRaProtocol::receive(LoRaPacket* packet) {
	int packetSize = this->lora->parsePacket();
	if (!packetSize) return;
	
	long i = 0;
	while (this->lora->available()) {
		packet->getData()[i++] = this->lora->read();
	}

	String decryptedData = Encryption::xorCipher(String(packet->getData()));
	packet->setData(decryptedData);

	processReceived(packet);
}

void LoRaProtocol::send(String message, uint hops) {
	while (ullmillis() < nextTxTime);

	static uint64_t msgLength = LoRaPacket::messageSize;
	uint64_t start = 0;
	long int dt = 0;
	while (message.length() > 0) {
		delay(dt);
		String first = message.substring(0, LoRaPacket::messageSize);
		message = message.substring(first.length());

		LoRaPacket* packet = new LoRaPacket();
		packet->setNew();
		packet->setMode(LoRaPacket::modeMSG);
		packet->setSrcId(getDeviceId());
		inventPacketId(packet);
		setHopCount(packet, hops);
		packet->setMessage((char*) first.c_str());
		dt = loraSend(packet);	
	}
}

void LoRaProtocol::relay(LoRaPacket* packet) {
	if (!CONFIG.isRelay()) return;
	if (CONFIG.isDebug()) Serial.println("RELAY: " + String(packet->getData()));
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
	ackPacket->setMessage((char*) String("ACK").c_str());

	loraSend(ackPacket);
	delete ackPacket;
}


void LoRaProtocol::processReceived(LoRaPacket* packet) {
	if (isIgnoredSender(packet->getSrcId())) return;

	if (packet->getMode() == LoRaPacket::modeACK) {
		if (sentPacketId.isEmpty()) return;
		if (packet->getPacketId() != sentPacketId) return;
		
		// here set setPacketId to "" because it's been ack'd
		sentPacketId = "";
		if (CONFIG.isDebug()) Serial.println("ACKED: " + packet->getPacketId());
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

void LoRaProtocol::setMessage(LoRaPacket* packet, char* message){
	packet->setMessage(message);
}

unsigned long LoRaProtocol::loraSend(LoRaPacket* packet) {
	String data = String(packet->getData());
	LoRaPacket* ackPacket = new LoRaPacket();
	unsigned long long expire;
	unsigned long long dt = 0;
	
	for (int i = 0; i < 3; i++) {
		if (CONFIG.isDebug()) Serial.println("SEND: " + data);
		
		dt = ullmillis();
		LoRa.beginPacket();
		LoRa.print(Encryption::xorCipher(data));
		LoRa.endPacket();
		if (packet->getMode() == LoRaPacket::modeACK) return dt - ullmillis();

		// Now wait for an ACK
		sentPacketId = packet->getPacketId();
		expire = ullmillis() + 3000;
		while (ullmillis() < expire) {
			receive(ackPacket);
			
			// sent packet is empty because it got ack'd
			if (sentPacketId.isEmpty()) {
				dt = ullmillis() - dt;
				nextTxTime = ullmillis() + dt;
				if (CONFIG.isDebug()) Serial.println("DONE: " + String((unsigned long) dt) + "ms");
				delete ackPacket;
				return dt;
			}
		}
	}

	sentPacketId = "";
	delete ackPacket;
	dt = ullmillis() - dt;
	Serial.println("***NO ACK***: " + packet->getPacketId() + " " + String((unsigned long) dt));
	return dt;
}


void LoRaProtocol::addSeen(char id[LoRaPacket::idSize + 1]) {
	strncpy(seen[currentSeen], id, LoRaPacket::idSize + 1); 
	currentSeen++;
	if (currentSeen >= SEEN_HISTORY) currentSeen = 0;
}

void LoRaProtocol::addFromMe(LoRaPacket* packet) {
	strncpy(fromMe[currentSeen], packet->getDataAtId(), LoRaPacket::idSize); 
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


