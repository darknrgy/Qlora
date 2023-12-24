#include "config.h"
#include <Arduino.h>

Config::Config() {
	for (long i = 0; i < 128; i++) {
		channels[i] = 902300000 + i * 200000;
	}
}

void Config::toggleDebug() {
	debug = !debug;
}

bool Config::isDebug() {
	return debug;
}

void Config::setBandwidth(long bandwidth) {
	Serial.println("Set bandwidth to " + String(bandwidth));
	this->bandwidth = bandwidth;
}

long Config::getBandwidth() {
	return bandwidth;
}

long Config::getChannelFrequency(long i) {
	return channels[i];
}
