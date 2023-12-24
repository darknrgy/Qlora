#include <SPI.h>
#include <LoRa.h>
#include "LoRaProtocol.h"
#include "config.h"
#include "ullmillis.h"
#include "config.h"

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

	lora.configure();
}

int ping = false;
unsigned long long expire = 0;
String command = "";

void loop() {
	String* reply;

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
				lora.send(userInput, LORA_HOPS);
			} else 	if (userInput.startsWith("//")) {
				// Run on the nearest node, but not on self (start remote ping for example)
				lora.send(userInput, 1);
				return;
			}

			runCmd(userInput);
			return;
		}
		
		Serial.println("<<< " + userInput);
		lora.send(userInput, LORA_HOPS);
	}

	if (ping) {
		if (ullmillis() > expire) {
			lora.send("PING", LORA_HOPS);
			expire = ullmillis() + 10000;
		}
	}
}


void runCmd(String userInput) {
	userInput.replace("/", "");
	String cmd = getNextCommandPart(&userInput);

	if (cmd == "ping") {
		ping = !ping;
		Serial.println("PING toggled");
	} else if (cmd == "debug") {
		Config::getInstance().toggleDebug();
	} else if (cmd == "blink") {
		for (int i = 0; i < 3; i++) {
			digitalWrite(LED_BUILTIN, LOW); delay(300); digitalWrite(LED_BUILTIN, HIGH); delay(300);
		}
	} else if (cmd == "set") {
		setConfig(userInput);
	}
}

void setConfig(String userInput) {
	String param = getNextCommandPart(&userInput);
	String value = getNextCommandPart(&userInput);

	if (param == "bandwidth") {
		lora.lora->setSignalBandwidth(value.toInt());
		Serial.println("Set signal bandwidth to " + value);
	} else if (param == "channel") {
		long channel = value.toInt();
		if (channel < 1 || channel > 128) {
			Serial.println("Channel must be between 1 and 128");
			return;
		}
		long frequency = Config::getInstance().getChannelFrequency(channel - 1);
		lora.lora->setFrequency(frequency);
		Serial.println("Channel is set to " + value + ": " + String(frequency));
	} else if (param == "power") {
		long power = value.toInt();
		if (power < 1 || power > 20) {
			Serial.println("Power must be between 1 and 20");
			return;
		}
		lora.lora->setTxPower(power);
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
