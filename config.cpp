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
	bw = prefs.getLong("bw");
	power = prefs.getLong("power");
	channel = prefs.getLong("channel");
	hops = prefs.getLong("hops");
	name = prefs.getString("name");
	ignore = prefs.getString("ignore");

	prefs.end();

	lora->configure();
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

void Config::setBandwidth(long bw) {
	this->bw = bw;
	lora->lora->setSignalBandwidth(bw);
	Serial.println("Set signal bw to " + String(bw));
	save();
}

long Config::getBandwidth() {
	return bw;
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
	return channels[channel - 1];
}

void Config::setHops(long hops){
	if (hops < 1 || hops > 20) {
		Serial.println("Hops must be between 1 and 20");
		return;
	}
	this->hops = hops;
	save();
}

long Config::getHops(){
	return hops;
}

void Config::setName(String name){
	if (name.length() < 1 || name.length() > 12) {
		Serial.println("Name must be between 1 and 12 chars");
		return;
	}
	this->name = name;
	save();
}

String Config::getName(){
	return name;
}

void Config::setIgnore(String ignore) {
	if (ignore.length() < 1 || ignore.length() > 26) {
		Serial.println("Ignore must be 3 ids at most");
	}
	this->ignore = ignore;
	save();
}

String Config::getIgnore() {
	return ignore;
}


void Config::setDefaults() {
	prefs.clear();
	prefs.putLong("version", PREFERENCES_VERSION);
	prefs.putBool("debug", true);
	prefs.putBool("relay", false);
	prefs.putLong("bw", 125E3);
	prefs.putLong("power", 1);
	prefs.putLong("channel", 64);
	prefs.putLong("hops", LORA_HOPS);
	prefs.putString("ignore", "");
	prefs.putString("name", "default");
}

void Config::save() {
	prefs.begin(PREFERENCES_NAMESPACE, false);
	
	prefs.putBool("debug", debug);
	prefs.putBool("relay", relay);
	prefs.putLong("bw", bw);
	prefs.putLong("power", power);
	prefs.putLong("channel", channel);
	prefs.putLong("hops", hops);
	prefs.putString("name", name);
	prefs.putString("ignore", ignore);

	prefs.end();
	Serial.println("Configuration saved");
}

String Config::getAllAsString() {
	String s;
	s += "CONFIG: ";
	s += "debug: " + String(debug) + ", ";
	s += "relay: " + String(relay) + ", ";
	s += "bw: " + String(bw) + ", ";
	s += "power: " + String(power) + ", ";
	s += "channel: " + String(channel) + ", ";
	s += "hops: " + String(hops) + ", ";
	s += "name: " + name + ", ";
	s += "ignore: " + ignore;

	return s;
}
