#include <Arduino.h>
#include "WebServerInterface.h"
#include "util.h"
#include "web-content.h"

const int bufferSize    = 100;                       // Number of lines in the buffer
const int maxLineLength = 128;                       // Maximum length of each line, including the null terminator
char      circularBuffer[bufferSize][maxLineLength]; // Circular buffer to hold the lines
int       currentIndex  = 0;                         // Current index in the buffer
int       lastSentIndex = 0;


void      handleUserInput(char* userInput);
String    getLinesAsJson();
WebServer server(80);

void WebServerInterface::listWifi()
{
	WiFi.scanDelete();
	int networkCount = WiFi.scanNetworks(false, true, false);
    
    for(int i=0; i<networkCount; ++i) {
    	char buffer[256];
    	sprintf(buffer, "%d) \"%s\" channel (%d) rssi (%d)", i, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i));
    	serialPrintln(buffer);
    }
}

void WebServerInterface::startWifi()
{
	if (wifiRunning) {
		serialPrintln("Wifi is already running.");
		return;
	}

	// Initialize the buffer with empty strings
    for (int i = 0; i < bufferSize; i++) {
        circularBuffer[i][0] = '\0'; // Set the first character to null terminator
    }

    const char* ssid     = CONFIG.getSSID();
    const char* password = CONFIG.getPassword();

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	// Wait for connection
	int maxTries = 30;
	while (WiFi.status() != WL_CONNECTED && maxTries > 0) {
		delay(500);
		serialPrint(".");
		--maxTries;
	}	

	if (maxTries == 0) {
		serialPrintln("Failed to connect.");
		return;
	}

	wifiRunning = true;

	printStatus();
}

void WebServerInterface::printStatus()
{
	if (wifiRunning) {
		serialPrintln("");
		serialPrint("Connected to: "); serialPrintln( CONFIG.getSSID()                      );
		serialPrint("Hostname:     "); serialPrintln( WiFi.getHostname()                    );
		serialPrint("IP address:   "); serialPrintln( WiFi.localIP().toString().c_str()     );
		serialPrint("Mac address:  "); serialPrintln( WiFi.macAddress().c_str()             );
		serialPrint("Subnet mask:  "); serialPrintln( WiFi.subnetMask().toString().c_str()  );
		serialPrint("Gateway IP:   "); serialPrintln( WiFi.gatewayIP().toString().c_str()   );
		serialPrint("DNS IP:       "); serialPrintln( WiFi.dnsIP().toString().c_str()       );
		serialPrint("Broadcast IP: "); serialPrintln( WiFi.broadcastIP().toString().c_str() );
		serialPrint("Network ID:   "); serialPrintln( WiFi.networkID().toString().c_str()   );
		serialPrint("Local IPV6:   "); serialPrintln( WiFi.localIPv6().toString().c_str()   );
	} else {
		serialPrintln("Wifi Disconnected");	
	}
}

void WebServerInterface::stopWifi()
{
	if (!wifiRunning) {
		serialPrintln("Wifi is already shut down.");
		return;
	}

	WiFi.disconnect();

	wifiRunning = false;
}


void WebServerInterface::startServer()
{
	server.on("/", []() {
		server.send(200, "text/html", html_content);
	});

	server.on("/style.min.css", []() {
		server.send(200, "text/css", css_content);
	});

	server.on("/script.min.js", []() {
		server.send(200, "text/javascript", js_content);
	});
	
	server.on("/submit", HTTP_POST, []() {
		if (server.hasArg("plain")) {

			char userData[1024];
			strncpy(userData, server.arg("plain").c_str(), 1023);
			userData[1023] = '\0';

			serialPrintln(userData); // Do something with postData
			handleUserInput(userData);
		}
		server.send(200, "text/plain", "success");
	});

	server.on("/poll", []() {
		server.send(200, "application/json", getLinesAsJson());
	});

	//ElegantOTA.begin(&server);    // Start ElegantOTA
	server.begin();
	serialPrintln("HTTP server started");

	serverRunning = true;
}

void WebServerInterface::stopServer()
{
	serverRunning = false;
	server.stop();
}

void WebServerInterface::update() {
	if (serverRunning) {
		server.handleClient();
	}
  	//ElegantOTA.loop();
}


// Function to convert the lines from the circular buffer to a JSON array string
String getLinesAsJson() {
	if (lastSentIndex == currentIndex)
		return "[]";

    String json = "[";

    // Start from the lastSentIndex up to currentIndex
    int i = lastSentIndex;
    do {
        json += "\"";
        json += String(circularBuffer[i]);
        json += "\",";
        i = (i + 1) % bufferSize;
    } while (i != currentIndex);

    if (json.endsWith(",")) {
        json.remove(json.length() - 1); // Remove the trailing comma
    }
    
    json += "]";
    lastSentIndex = currentIndex; // Update lastSentIndex to the currentIndex
    return json;
}


void addToBuffer(const char* text, bool newLine = true) {
    const char* currentLine = text;
    while (*currentLine != '\0') { // Iterate through the string
        // Find the next new line character or end of the string
        const char* nextLine = strchr(currentLine, '\n');
        int lineLength = nextLine != nullptr? nextLine - currentLine : strlen(currentLine);

        // Append the current line to the circular buffer
        int copyLength = (lineLength < maxLineLength - 1) ? lineLength : maxLineLength - 1;
        strncat(circularBuffer[currentIndex], currentLine, copyLength);
        circularBuffer[currentIndex][maxLineLength - 1] = '\0';

        if (nextLine == nullptr) break;

    	currentIndex = (currentIndex + 1) % bufferSize;
        circularBuffer[currentIndex][0] = '\0';
        currentLine = nextLine + 1; // Move to the start of the next line
    }

    // Move to the next index if newLine is true or end of the line reached
    if (newLine) {
        currentIndex = (currentIndex + 1) % bufferSize;
        // Clear the next line
        circularBuffer[currentIndex][0] = '\0';
    }
}


void serialPrint(const char* text)
{
	addToBuffer(text, false);
	Serial.print(text);
}

void serialPrintln(const char* line)
{
	addToBuffer(line);
	Serial.println(line);
}

