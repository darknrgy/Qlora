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
	bandwidth = prefs.getLong("bw");
	power = prefs.getLong("power");
	channel = prefs.getLong("channel");
	hops = prefs.getLong("hops");
	
	strncpy(name, prefs.getString("name").c_str(), maxNameSize-1);
	name[maxNameSize-1] = '\0';

	strncpy(ignore, prefs.getString("ignore").c_str(), maxIgnoreSize-1);
	ignore[maxIgnoreSize-1] = '\0';

	prefs.end();

	lora->configure();
}

void Config::setLora(LoRaProtocol* lora) {
	this->lora = lora;
}

void Config::toggleDebug() {
	debug = !debug;
	SERIAL_LOG_FORMAT(64, "Debug set to %s", debug ? "true" : "false");
	save();
}

bool Config::isDebug() {
	return debug;
}

void Config::setBandwidth(long bandwidth) {
	this->bandwidth = bandwidth;
	lora->lora->setSignalBandwidth(bandwidth);
	SERIAL_LOG_FORMAT(64, "Set signal bandwidth to %ld", bandwidth);
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
	SERIAL_LOG_FORMAT(64, "Relay set to %s", relay ? "true" : "false");
	save();
}

bool Config::isRelay() {
	return relay;
}

void Config::setPower(long power) {
	if (power < 1 || power > 20) {
		serialPrintln("Power must be between 1 and 20");
		return;
	}
	this->power = power;
	lora->lora->setTxPower(power);
	SERIAL_LOG_FORMAT(64, "Power is set to %ld", power);
	save();
}

long Config::getPower() {
	return power;
}

void Config::setChannel(long channel) {
	if (channel < 1 || channel > 128) {
		serialPrintln("Channel must be between 1 and 128");
		return;
	}
	this->channel = channel;
	long frequency = getChannelFrequency(channel - 1);
	lora->lora->setFrequency(frequency);

	SERIAL_LOG_FORMAT(64, "Channel is set to %ld: %ld", channel, frequency);
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
		serialPrintln("Hops must be between 1 and 20");
		return;
	}
	this->hops = hops;
	save();
}

long Config::getHops(){
	return hops;
}

void Config::setName(const char* name) {
    size_t nameLength = strlen(name);
    if (nameLength < 1 || nameLength > maxNameSize) {
        serialPrintln("Name must be between 1 and 127 chars");
        return;
    }
    strncpy(this->name, name, maxNameSize-1);
    this->name[maxNameSize-1] = '\0'; // Ensure null termination

    save();
}

const char* Config::getName(){
	return name;
}

void Config::setIgnore(const char* ignore) {
	int len = strnlen(ignore, maxIgnoreSize-1);
	if (len < 1 || len > maxIgnoreSize-1) {
		serialPrintln("Ignore must be 15 ids at most");
	}
	
	strncpy(this->ignore, ignore, maxIgnoreSize-1);
	this->ignore[maxIgnoreSize-1] = '\0';

	save();
}

const char* Config::getIgnore() {
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
	prefs.putLong("bw", bandwidth);
	prefs.putLong("power", power);
	prefs.putLong("channel", channel);
	prefs.putLong("hops", hops);
	prefs.putString("name", name);
	prefs.putString("ignore", ignore);

	prefs.end();
	serialPrintln("Configuration saved");
}

void Config::getAllAsString(char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize,
             "CONFIG: debug: %s, relay: %s, bandwidth: %d, power: %d, channel: %d, hops: %d, name: %s, ignore: %s",
             debug ? "true" : "false", 
             relay ? "true" : "false", 
             bandwidth, power, channel, hops,
             name, ignore);
}

