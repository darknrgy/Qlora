#include <SPI.h>
#include <LoRa.h>
#include "LoRaProtocol.h"
#include "config.h"
#include "ullmillis.h"
#include "util.h"
#include "sleep.h"

LoRaProtocol lora(&LoRa);
Sleep        sleepManager(&LoRa);


static const size_t maxUserInput = 512;


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
    static char reply[LoRaPacket::packetSize];
    static unsigned long long nextPingTime = 0;

    if (lora.listenAndRelay()) sleepManager.extendAwake();

    strncpy(reply, lora.getLastReply(), LoRaPacket::packetSize-1);
    reply[LoRaPacket::packetSize - 1] = '\0'; // Ensure null termination

    // Handle remote config changes
    if (strncmp(reply, "//", 2) == 0) {
        replaceNewlineWithSpace(reply);

        if (strncmp(reply, "////", 4) == 0) {
            if (strncmp(reply + 4, getDeviceId(), 8) == 0) {
                // This command is addressed to me
                runCmd(reply + 13);
            }
            return;
        }

        runCmd(reply);
        return;
    }

    if (Serial.available() > 0) {
        sleepManager.extendAwake();
        char userInput[maxUserInput] = {0}; // Initialize with all zeros
        int userInputLength = 0;

        while (true) {
            while (Serial.available() > 0 && userInputLength < maxUserInput - 1) {
                char c = Serial.read();
                if (c != -1) {
                    if ((int)c < 0x01 || (int)c > 0x7F) {
                        Serial.print("Character ");
                        Serial.print(c);
                        Serial.println(" omitted; chars must be ascii");
                    } else {
                        userInput[userInputLength++] = c;
                    }
                } else {
                    delay(2);
                }
            }
            delay(20);
            if (Serial.available() == 0) break;
        }
        userInput[userInputLength] = '\0'; // Null-terminate the string
        replaceNewlineWithSpace(userInput);

        if (userInput[0] == '/') {
            if (strncmp(userInput, "////", 4) == 0) {
                lora.send(userInput, CONFIG.getHops());
                return;
            }

            if (strncmp(userInput, "///", 3) == 0) {
                lora.send(userInput, CONFIG.getHops());
                runCmd(userInput);
                return;
            }

            if (strncmp(userInput, "//", 2) == 0) {
                lora.send(userInput, 1);
                runCmd(userInput);
                return;
            }

            runCmd(userInput);
            return;
        }


        char message[maxUserInput + 50]; // To accommodate additional text
        sprintf(message, "<<< %s: %s", CONFIG.getName(), userInput);
        Serial.println(message);
        lora.send(message, CONFIG.getHops());
    }

    ping(-1);
    sleepManager.sleepIfShould();
}

// Helper function to replace newline with space
void replaceNewlineWithSpace(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            str[i] = ' ';
        }
    }
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
		
		char voltageString[128]; // Stack allocation (temporary)
		sprintf(voltageString, "Voltage %s: BANK1: %.2f BANK2: %.2f", getDeviceId(), bank1, bank2);
		Serial.println(voltageString);

		lora.send(voltageString, CONFIG.getHops());
	} else if (cmd == "get") {
		String deviceID = getNextCommandPart(&userInput);
		const char* myDeviceId = getDeviceId();

		char send[255]; // Stack allocation (temporary)
		sprintf(send, "%s: %s", myDeviceId, CONFIG.getAllAsString().c_str());

		if (!deviceID.isEmpty()) {			
			// strcmp returns 0 when strings are equal
			if (!strcmp(deviceID.c_str(),myDeviceId)) { 
				lora.send(send, CONFIG.getHops()); // Temp, this will change to const char*
			}
			return;
		}

		Serial.println(send);
	} else if (cmd == "gain") {
		long gain = getNextCommandPart(&userInput).toInt();
		lora.lora->setGain(gain);
		Serial.println("Gain set to " + String(gain));
	} else if (cmd == "help") {
		Serial.println("\nHELP (list of all commands):");
		Serial.println("/get (list all saved configs)");
		Serial.println("/set <param> <value>");
		Serial.println("//set <param> <value> (set a param to any first hop listening)");
		Serial.println("////<deviceId> set <param> <value> (set a param on any device)");
		Serial.println("///get [deviceId] (get config of remote device)");
		
		Serial.println("\nPARAMS:");
		Serial.println("bandwidth: 125000, 250000");
		Serial.println("power: 1 through 20");
		Serial.println("channel: 1 through 128");
		Serial.println("hops: 1 through 255");
		Serial.println("name: 1 through 8 chars");
		Serial.println("ignore: Ignore up to 3 comma separated deviceIds\n");

		Serial.println("/debug (toggle debug)");
		Serial.println("/relay (toggle relay)");
		Serial.println("/ping (enable ping)");
		Serial.println("/unping (disable ping)");
		Serial.println("/gain 0 - 6 (not saved with config");
		Serial.println("In general: / (me), // (next hop), /// (every device), ////<deviceID> (addressed to device");
		Serial.println("Caution: You can get into trouble by doing things like ///ping, which will permanently flood the network");
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
			nextPingTime = ullmillis() + 10000;
			Serial.println("<<< PING");
			lora.send("PING", CONFIG.getHops());
			sleepManager.extendAwake();			
		} 
	}
}

