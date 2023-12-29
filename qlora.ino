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

void runCmd(char* userInput) {
    // Remove all '/' characters from the beginning of userInput
    char* cmdStart = userInput;
    while (*cmdStart == '/') {
        cmdStart++;
    }

    char cmd[maxUserInput];
    getNextCommandPart(cmdStart, cmd); // Extract the first command part

    if (strcmp(cmd, "ping") == 0) {

        ping(1);
        Serial.println("PING enabled");

    } else if (strcmp(cmd, "unping") == 0) {

        ping(0);
        Serial.println("PING disabled");

    } else if (strcmp(cmd, "debug") == 0) {

        CONFIG.toggleDebug();

    } else if (strcmp(cmd, "blink") == 0) {
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_BUILTIN, LOW); delay(300); digitalWrite(LED_BUILTIN, HIGH); delay(300);
        }
    } else if (strcmp(cmd, "set") == 0) {

        setConfig(cmdStart);

    } else if (strcmp(cmd, "relay") == 0) {

        CONFIG.toggleRelay();

    } else if (strcmp(cmd, "voltage") == 0) {

        float bank1 = getBatteryVoltage(VOLTAGE_READ_PIN0);
        float bank2 = getBatteryVoltage(VOLTAGE_READ_PIN1);
        bank1 -= bank2;
        
        char voltageString[128]; // Stack allocation (temporary)
        sprintf(voltageString, "Voltage %s: BANK1: %.2f BANK2: %.2f", getDeviceId(), bank1, bank2);
        Serial.println(voltageString);

        lora.send(voltageString, CONFIG.getHops());

    } else if (strcmp(cmd, "get") == 0) {

		char deviceID[LoRaPacket::idSize + 1]; 
		getNextCommandPart(cmdStart, deviceID);
		deviceID[LoRaPacket::idSize] = '\0';

		char config[maxUserInput];
		CONFIG.getAllAsString(config, maxUserInput-1);

		char send[maxUserInput]; // Stack allocation (temporary)
		snprintf(send, maxUserInput-1, "%s: %s", getDeviceId(), config);
		send[maxUserInput-1] = '\0';

		if (deviceID[0] != '\0') { // If not an empty string
			// strcmp returns 0 when strings are equal
			if (!strcmp(deviceID, getDeviceId())) { 
				lora.send(send, CONFIG.getHops()); // Temp, this will change to const char*
			}
			return;
		}

		Serial.println(send);

	} else if (strcmp(cmd, "gain") == 0) {

        char gainStr[maxUserInput];
        getNextCommandPart(cmdStart, gainStr);
        long gain = atol(gainStr);
        lora.lora->setGain(gain);
        SERIAL_LOG_FORMAT(64, "Gain set to %ld", &gain);

    } else if (strcmp(cmd, "help") == 0) {

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
		SERIAL_LOG_FORMAT(128, "Unrecognized command: %s Type /help for help", cmd);
	}
}

void setConfig(char* userInput) {
    char param[maxUserInput];
    getNextCommandPart(userInput, param);

    char value[maxUserInput];
    getNextCommandPart(userInput, value);

    if (strcmp(param, "bandwidth") == 0) {
        CONFIG.setBandwidth(atoi(value));
    } else if (strcmp(param, "channel") == 0) {
        CONFIG.setChannel(atoi(value));      
    } else if (strcmp(param, "power") == 0) {
        CONFIG.setPower(atoi(value));
    } else if (strcmp(param, "hops") == 0) {
        CONFIG.setHops(atoi(value));
    } else if (strcmp(param, "name") == 0) {
        CONFIG.setName(value);
    } else if (strcmp(param, "ignore") == 0) {
        CONFIG.setIgnore(value);
    }
}


void getNextCommandPart(char* input, char* nextPart) {
    // Find the first space in the input
    char* spacePtr = strchr(input, ' ');
    if (spacePtr == NULL) {
        // No space found, copy the whole input to nextPart
        strncpy(nextPart, input, maxUserInput - 1);
        nextPart[maxUserInput - 1] = '\0'; // Ensure null termination
        input[0] = '\0'; // Clear the input
        return;
    }

    // Calculate the length of the next part
    int nextPartLength = spacePtr - input;
    strncpy(nextPart, input, nextPartLength);
    nextPart[nextPartLength] = '\0'; // Ensure null termination

    // Shift the remainder of input to the beginning
    memmove(input, spacePtr + 1, strlen(spacePtr + 1) + 1); // +1 to include the null terminator
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

