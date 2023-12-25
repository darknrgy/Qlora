#include "config.h"
#include "LoRaProtocol.h"

Config::Config() {
	// Calculate all frequencies based on channels 1-128
	for (long i = 0; i < 128; i++) {
		channels[i] = 902300000 + i * 200000;
	}	
}

void Config::load() {
	prefs.begin(PREFERENCES_NAMESPACE, false);
	int version = prefs.getLong("version");
	if (version != PREFERENCES_VERSION) {
		setDefaults();
	} 

	debug = prefs.getBool("debug");
	relay = prefs.getBool("relay");
	bandwidth = prefs.getLong("bandwidth");
	power = prefs.getLong("power");
	channel = prefs.getLong("channel");

	prefs.end();
}

void Config::setLora(LoRaProtocol* lora) {
	this->lora = lora;
}

void Config::toggleDebug() {
	debug = !debug;
	Serial.println("Debug set to " + String(debug));
	save();
}

bool Config::isDebug() {
	return debug;
}

void Config::setBandwidth(long bandwidth) {
	this->bandwidth = bandwidth;
	lora->lora->setSignalBandwidth(bandwidth);
	Serial.println("Set signal bandwidth to " + String(bandwidth));
	save();
}

long Config::getBandwidth() {
	return bandwidth;
}

long Config::getChannelFrequency(long i) {
	return channels[i];
}

void Config::toggleRelay() {
	relay = !relay;
	Serial.println("Relay set to " + String(relay));
	save();
}

bool Config::isRelay() {
	return relay;
}

void Config::setPower(long power) {
	if (power < 1 || power > 20) {
		Serial.println("Power must be between 1 and 20");
		return;
	}
	this->power = power;
	lora->lora->setTxPower(power);
	Serial.println("Power is set to " + String(power));
	save();
}

long Config::getPower() {
	return power;
}

void Config::setChannel(long channel) {
	if (channel < 1 || channel > 128) {
		Serial.println("Channel must be between 1 and 128");
		return;
	}
	this->channel = channel;
	long frequency = getChannelFrequency(channel - 1);
	lora->lora->setFrequency(frequency);
	Serial.println("Channel is set to " + String(channel) + ": " + String(frequency));
	save();
}

long Config::getChannel() {
	return channel;
}

long Config::getFrequency() {
	return channels[channel];
}

void Config::setDefaults() {
	prefs.clear();
	prefs.putLong("version", PREFERENCES_VERSION);
	prefs.putBool("debug", true);
	prefs.putBool("relay", true);
	prefs.putLong("bandwidth", 125E3);
	prefs.putLong("power", 1);
	prefs.putLong("channel", 64);
}

void Config::save() {
	prefs.begin(PREFERENCES_NAMESPACE, false);
	
	prefs.putBool("debug", debug);
	prefs.putBool("relay", relay);
	prefs.putLong("bandwidth", bandwidth);
	prefs.putLong("power", power);
	prefs.putLong("channel", channel);

	prefs.end();
	Serial.println("Configuration saved");
}

String Config::getAllAsString() {
	String s;
	s += "ACTIVE CONFIG: \n";
	s += "debug: " + String(debug) + "\n";
	s += "relay: " + String(relay) + "\n";
	s += "bandwidth: " + String(bandwidth) + "\n";
	s += "power: " + String(power) + "\n";
	s += "channel: " + String(channel) + "\n";
	return s;
}
