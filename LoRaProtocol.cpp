#include "LoRaProtocol.h"

// Public LoRaProtocol

LoRaProtocol::LoRaProtocol(LoRaClass* lora) {
	this->lora = lora;
}

void LoRaProtocol::listenAndRelay() {
	lastReply = "";
	LoRaPacket* packet = new LoRaPacket();
	receive(packet);

	if (packet->isNew()) {
		lastReply = String(packet->getMessage());
		
		if (Config::getInstance().isDebug()) {
			Serial.print("RSSI " + String(LoRa.packetRssi()) + " ");
		}

		Serial.println(">>> " + lastReply);
	}

	if (packet->isRelay()) {
		relay(packet);
	}

	delete packet;
}

String* LoRaProtocol::getLastReply() {
	return &lastReply;
}

void LoRaProtocol::receive(LoRaPacket* packet) {
	int packetSize = this->lora->parsePacket();
	if (!packetSize) return;
	
	long i = 0;
	while (this->lora->available()) {
		packet->getData()[i++] = this->lora->read();
	}

	processReceived(packet);
}

void LoRaProtocol::send(String message, uint hops) {
	while (ullmillis() < nextTxTime);

	static uint64_t msgLength = LoRaPacket::messageSize;
	uint64_t start = 0;
	while (message.length() > 0) {
		String first = message.substring(0, LoRaPacket::messageSize);
		message = message.substring(first.length());

		LoRaPacket* packet = new LoRaPacket();
		packet->setNew();
		packet->setMode(LoRaPacket::modeMSG);
		inventPacketId(packet);
		setHopCount(packet, hops);
		packet->setMessage((char*) first.c_str());

		long int dt = loraSend(packet);
		delay(dt);
	}
}

void LoRaProtocol::relay(LoRaPacket* packet) {
	if (!Config::getInstance().isRelay()) return;
	if (Config::getInstance().isDebug()) Serial.println("RELAY: " + String(packet->getData()));
	addFromMe(packet);
	loraSend(packet);
}

// Private LoRaProtocol
void LoRaProtocol::sendAckPacket(LoRaPacket* packet) {
	delay(random(100,1000));
	LoRaPacket* ackPacket = new LoRaPacket();
	ackPacket->setMode(LoRaPacket::modeACK);
	ackPacket->setPacketId(packet->getData());
	setHopCount(ackPacket, 0);
	ackPacket->setMessage((char*) String("ACK").c_str());

	loraSend(ackPacket);
	delete ackPacket;
}


void LoRaProtocol::processReceived(LoRaPacket* packet) {
	if (packet->getMode() == LoRaPacket::modeACK) {
		if (sentPacketId.isEmpty()) return;
		if (packet->getPacketId() != sentPacketId) return;
		sentPacketId = "";
		if (Config::getInstance().isDebug()) Serial.println("ACKED: " + packet->getPacketId());
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
	strncpy(id, packet->getData(), LoRaPacket::idSize);
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
	strncpy(id, packet->getData(), LoRaPacket::idSize);
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
    strncpy(hopStr, packet->getData() + LoRaPacket::idSize, LoRaPacket::hopSize);
    hopStr[LoRaPacket::hopSize] = '\0';

    int hopCount = atoi(hopStr);
    hopCount = hopCount > 0 ? hopCount - 1 : 0;

    setHopCount(packet, hopCount);
    return hopCount;
}

void LoRaProtocol::setHopCount(LoRaPacket* packet, uint64_t hopCount) {
    char hopStr[LoRaPacket::hopSize + 1];
    snprintf(hopStr, LoRaPacket::hopSize + 1, "%0*llu", LoRaPacket::hopSize, hopCount);
    memcpy(packet->getData() + LoRaPacket::idSize, hopStr, LoRaPacket::hopSize);
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
		if (Config::getInstance().isDebug()) Serial.println("SEND: " + data);
		
		dt = ullmillis();
		LoRa.beginPacket();
		LoRa.print(data);
		LoRa.endPacket();
		if (packet->getMode() == LoRaPacket::modeACK) return dt - ullmillis();

		// Now wait for an ACK
		sentPacketId = packet->getPacketId();
		expire = ullmillis() + 2500;
		while (ullmillis() < expire) {
			receive(ackPacket);
			if (sentPacketId.isEmpty()) {
				dt = ullmillis() - dt;
				nextTxTime = ullmillis() + dt;
				if (Config::getInstance().isDebug()) Serial.println("DONE: " + String((unsigned long) dt) + "ms");
				delete ackPacket;
				return dt;
			}
		}
	}

	sentPacketId = "";
	delete ackPacket;
	dt = ullmillis() - dt;
	if (Config::getInstance().isDebug()) Serial.println("***NO ACK***: " + packet->getPacketId() + " " + String((unsigned long) dt));
	return dt;
}


void LoRaProtocol::addSeen(char id[LoRaPacket::idSize + 1]) {
	strncpy(seen[currentSeen], id, LoRaPacket::idSize + 1); 
	currentSeen++;
	if (currentSeen >= SEEN_HISTORY) currentSeen = 0;
}

void LoRaProtocol::addFromMe(LoRaPacket* packet) {
	strncpy(fromMe[currentSeen], packet->getData(), LoRaPacket::idSize); 
	fromMe[currentSeen][LoRaPacket::idSize] = '\0';
	currentFromMe++;
	if (currentFromMe >= SEEN_HISTORY) currentFromMe = 0;
}

/*void LoRaProtocol::configure() {
	this->lora->setTxPower(1);
}*/



void LoRaProtocol::configure() {
	this->lora->enableInvertIQ();
	this->lora->setSpreadingFactor(12);
	this->lora->setSignalBandwidth(CONFIG.getBandwidth());
	this->lora->setCodingRate4(8);
	this->lora->setTxPower(CONFIG.getPower());
	this->lora->setFrequency(CONFIG.getFrequency());
}


