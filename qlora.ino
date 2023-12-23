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
	
	lora.listenAndRelay();

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

		command = userInput;
		command.replace("\n", "");
		userInput.replace("\n", " ");

		
		if (command == "/ping") {
			Serial.println("Ping enabled");
			ping = true;
		}

		if (command == "/debug") {
			Config::getInstance().toggleDebug();
		}

		Serial.println(">>> " + userInput);
		
		lora.send(userInput, 2);
	}

	if (ping) {
		if (ullmillis() > expire) {
			lora.send("PING", 2);
			expire = ullmillis() + 10000;
		}
	}
}
