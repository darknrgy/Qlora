#include <SPI.h>
#include <LoRa.h>
#include "LoRaProtocol.h"
#include "config.h"
#include "ullmillis.h"
#include "util.h"

LoRaProtocol lora(&LoRa);

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	
	Serial.begin(115200);

	LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);

	while (!LoRa.begin(915E6)) {
		Serial.println("Starting LoRa failed!");
		delay(1000);
	}

	CONFIG.setLora(&lora);
	CONFIG.load();
}

void loop() {
	String* reply;
	static unsigned long long nextPingTime = 0;

	lora.listenAndRelay();

	reply = lora.getLastReply();
	
	// Handle remote config changes
	if (reply->startsWith("//")) {
		reply->replace("\n", " ");
		runCmd(*reply);
		return;
	}

	if (Serial.available() > 0) {
		String userInput = "";
		
		while (true) {
			while (Serial.available() > 0) {			
				char c = Serial.read();
				if (c != -1) {
					userInput += c;
				} else {
					delay(2);
				}			
			}
			delay(20);
			if (Serial.available() == 0) break;
		}

		userInput.replace("\n", " ");

		if (userInput.startsWith("/")) {
			if (userInput.startsWith("///")) {
				// Run command on self and all other nodes
				lora.send(userInput, CONFIG.getHops());
			} else 	if (userInput.startsWith("//")) {
				// Run on the nearest node, but not on self (start remote ping for example)
				lora.send(userInput, 1);
				return;
			}

			runCmd(userInput);
			return;
		}
		
		Serial.println("<<< " + CONFIG.getName() + ": " + userInput);
		lora.send(CONFIG.getName() + ": " + userInput, CONFIG.getHops());
	}

	ping(-1);
}


void runCmd(String userInput) {
	userInput.replace("/", "");
	String cmd = getNextCommandPart(&userInput);

	if (cmd == "ping") {
		ping(1);
		Serial.println("PING enabled");
	} else if (cmd == "unping") {
		ping(0);
		Serial.println("PING disabled");
	} else if (cmd == "debug") {
		CONFIG.toggleDebug();
	} else if (cmd == "blink") {
		for (int i = 0; i < 3; i++) {
			digitalWrite(LED_BUILTIN, LOW); delay(300); digitalWrite(LED_BUILTIN, HIGH); delay(300);
		}
	} else if (cmd == "set") {
		setConfig(userInput);
	} else if (cmd == "relay") {
		CONFIG.toggleRelay();		
	} else if (cmd == "voltage") {
		float bank1 = getBatteryVoltage(VOLTAGE_READ_PIN0);
		float bank2 = getBatteryVoltage(VOLTAGE_READ_PIN1);
		bank1 -= bank2;
		
		String voltageString = "Voltage " + getDeviceId() + ": BANK1: " + String(bank1,2) + ", BANK2: " + String(bank2, 2);
		Serial.println(voltageString);
		lora.send(voltageString, CONFIG.getHops());
	} else if (cmd == "get") {
		String deviceID = getNextCommandPart(&userInput);
		String myDeviceId = getDeviceId();
		String send = myDeviceId + ": " + CONFIG.getAllAsString();
		if (!deviceID.isEmpty()) {			
			if (deviceID == myDeviceId) {
				lora.send(send, CONFIG.getHops());
			}
			return;
		}
		Serial.println(send);
	} else if (cmd == "gain") {
		long gain = getNextCommandPart(&userInput).toInt();
		lora.lora->setGain(gain);
		Serial.println("Gain set to " + String(gain));
	} else if (cmd == "help") {
		Serial.println("HELP (list of all commands)");
		Serial.println("/set <param> <value>");
		Serial.println("bandwidth: 125000, 250000");
		Serial.println("power: 1 through 20");
		Serial.println("channel: 1 through 128");
		Serial.println("/debug (toggle debug)");
		Serial.println("/relay (toggle relay)");
		Serial.println("/ping (enable ping)");
		Serial.println("/unping (disable ping)");
		Serial.println("/gain 0 - 6 (not saved with config");
		Serial.println("/get [deviceId] (get list of all configs)");
	} else {
		Serial.println("Unrecognized command: " + String(cmd) + " Type /help for help");
	}
}

void setConfig(String userInput) {
	String param = getNextCommandPart(&userInput);
	String value = getNextCommandPart(&userInput);

	if (param == "bandwidth") {
		CONFIG.setBandwidth(value.toInt());
	} else if (param == "channel") {
		CONFIG.setChannel(value.toInt());		
	} else if (param == "power") {
		CONFIG.setPower(value.toInt());
	} else if (param == "hops") {
		CONFIG.setHops(value.toInt());
	} else if (param == "name") {
		CONFIG.setName(value);
	} else if (param == "ignore") {
		CONFIG.setIgnore(value);
	}
}

String getNextCommandPart(String* input) {
	int spaceIndex = input->indexOf(" ");

	if (spaceIndex == -1) {
        return *input;
    }

	String next = input->substring(0, spaceIndex);
	input->remove(0, input->indexOf(" ") + 1);
	return next;
}

void ping(int enable) {
	static int enabled = 0;
	static unsigned long long nextPingTime = 0;
	if (enable == 0) enabled = 0;
	else if (enable == 1) {
		enabled = 1;
		nextPingTime = ullmillis();
	}
	
	if (enable == -1 && enabled == 1) {
		if (ullmillis() >= nextPingTime) {
			nextPingTime = ullmillis() + 5000;
			Serial.println("<<< PING");
			lora.send("PING", CONFIG.getHops());			
		} 
	}
}

