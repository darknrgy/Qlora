
#include <FS.h>
#include <SPIFFS.h>
#include <Arduino.h>

#include "WebServerInterface.h"
#include "util.h"
#include "web-content.h"

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true
#define MAX_FILENAME_LENGTH     256
#define JSON_BUFFER_SIZE        1024

const int circBufferSize  = 100;                         // Number of lines in the buffer
const int maxLineLength   = 128;                         // Maximum length of each line, including the null terminator
char      circularBuffer[circBufferSize][maxLineLength]; // Circular buffer to hold the lines
int       currentIndex    = 0;                           // Current index in the buffer
int       lastSentIndex   = 0;


void      handleUserInput(char* userInput);
void      handleFileUpload();
void      handleFileDownload();
char*     listFiles(char* jsonBuffer, int bufferSize);
void      deleteFile(const char* filename);
char*     getLinesAsJson(char* jsonBuffer, int bufferSize);
void      makeRootPath(char * file, const int bufferSize, const char* src);


WebServer server(80);

void WebServerInterface::listWifi()
{
	WiFi.scanDelete();
	int networkCount = WiFi.scanNetworks(false, true, false);
	
	for(int i=0; i<networkCount; ++i) {
		char buffer[256];
		sprintf(buffer, "%d) '%s' channel (%d) rssi (%d)", i, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i));
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
	for (int i = 0; i < circBufferSize; i++) {
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
	// Root
	server.on("/", []() {
		server.send(200, "text/html", html_content);
	});

	// Setup endpoint to serve css
	server.on("/style.min.css", []() {
		server.send(200, "text/css", css_content);
	});

	// Setup endpoint to serve javascript
	server.on("/script.min.js", []() {
		server.send(200, "text/javascript", js_content);
	});
	
	// Setup endpoint to process a web-based serial input
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

	// Setup endoing to get latest serial
	server.on("/poll", []() {
		char jsonBuffer[1024];
		server.send(200, "application/json", getLinesAsJson(jsonBuffer, sizeof(jsonBuffer)));
	});

	// Setup endpoint for file upload
	server.on("/upload", HTTP_POST, []() {
		server.send(200);
	}, handleFileUpload);

	// Setup endpoint to list files
	server.on("/list-files", HTTP_GET, [this]() {
		char jsonBuffer[1024];
		server.send(200, "application/json", listFiles(jsonBuffer, sizeof(jsonBuffer)));
	});

	// Setup endpoint to download a file
	server.on("/download", HTTP_GET, [this]() {
		handleFileDownload();
	});

	server.on("/delete-file", HTTP_POST, [this]() {
		SERIAL_LOG_FORMAT(128,"Delete: Has plain arg: %s", server.hasArg("plain")? "true" : "false");
		if (server.hasArg("plain")) {
			char filename[1024];
			strncpy(filename, server.arg("plain").c_str(), 1023);
			filename[1023] = '\0';
			serialPrintln(filename);
			deleteFile(filename);
			server.send(200, "text/plain", "File deleted");
		} else {
			server.send(400, "text/plain", "Bad Request");
		}
	});


	//ElegantOTA.begin(&server);    // Start ElegantOTA
	server.begin();
	if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
		serialPrintln("SPIFFS Mount Failed");
	}
	serialPrintln("HTTP server started");

	serverRunning = true;
}

void WebServerInterface::stopServer()
{
	serverRunning = false;
	SPIFFS.end();
	server.stop();
}

void WebServerInterface::update() {
	if (serverRunning) {
		server.handleClient();
	}
	//ElegantOTA.loop();
}



void makeRootPath(char * file, const int bufferSize, const char* src)
{
	if (src[0] != '/') snprintf(file, bufferSize, "/%s", src);	
	else               strncpy(file, src, bufferSize - 1);

	file[bufferSize - 1] = '\0'; // Ensure null termination
}

void handleFileUpload() {
	HTTPUpload& upload = server.upload();
	
	char filename[MAX_FILENAME_LENGTH];
	makeRootPath(filename, MAX_FILENAME_LENGTH, upload.filename.c_str());

	//@TODO - potentially put these files in an /upload folder

	if (upload.status == UPLOAD_FILE_START) {
		SERIAL_LOG_FORMAT(128, "Uploading file: %s", filename);
		SPIFFS.remove(filename); // Overwrite existing file

	} else if (upload.status == UPLOAD_FILE_WRITE) {
		File file = SPIFFS.open(filename, FILE_APPEND);
		if (file) {
			SERIAL_LOG_FORMAT(128, "File appended, wrote %d", upload.currentSize);
			file.write(upload.buf, upload.currentSize);
			file.close();
		}
	}
}

void handleFileDownload() {
    if (server.hasArg("filename")) {
        char filePath[MAX_FILENAME_LENGTH];
        makeRootPath(filePath, MAX_FILENAME_LENGTH, server.arg("filename").c_str());

        SERIAL_LOG_FORMAT(128, "Download: %s", filePath);

        if (!SPIFFS.exists(filePath)) {
            SERIAL_LOG_FORMAT(128, "Download: File not found %s", filePath);
            server.send(404, "text/plain", "File not found");
            return;
        }

        File file = SPIFFS.open(filePath, "r");
        if (!file || file.isDirectory()) {
            SERIAL_LOG_FORMAT(128, "Download: Failed to open file %s", filePath);
            server.send(500, "text/plain", "Internal Server Error");
            return;
        }

        const char* contentType = "application/octet-stream";
        server.setContentLength(file.size());

        char contentDisposition[256];
        snprintf(contentDisposition, sizeof(contentDisposition), "attachment; filename=%s", filePath + 1);
        server.sendHeader("Content-Disposition", contentDisposition);

        server.streamFile(file, contentType);
        file.close();
    } else {
        server.send(400, "text/plain", "Bad Request");
    }
}

char* listFiles(char* jsonBuffer, int bufferSize) {
	strcpy(jsonBuffer, "[");

	File root = SPIFFS.open("/");
	if (!root || !root.isDirectory()) {
		strcat(jsonBuffer, "]");
		return jsonBuffer;
	}

	File file = root.openNextFile();
	while (file) {
		if (strlen(jsonBuffer) > 1) strcat(jsonBuffer, ",");
		char fileEntry[128];
		snprintf(fileEntry, sizeof(fileEntry), "{\"name\":\"%s\", \"size\":\"%d\"}", file.name(), file.size());
		strncat(jsonBuffer, fileEntry, bufferSize);
		file = root.openNextFile();
	}
	strcat(jsonBuffer, "]");
	return jsonBuffer;
}

void deleteFile(const char* filename) {
	char filePath[MAX_FILENAME_LENGTH];
	makeRootPath(filePath, MAX_FILENAME_LENGTH, filename);
	SPIFFS.remove(filePath);
}


// Function to convert the lines from the circular buffer to a JSON array string
char* getLinesAsJson(char* jsonBuffer, int bufferSize) {
	
	strcpy(jsonBuffer, "[");

	if (lastSentIndex == currentIndex)
		return "[]";

	int i = lastSentIndex;
	do {
		char lineBuffer[128]; // Assuming each line is not longer than 128 characters
		snprintf(lineBuffer, sizeof(lineBuffer), "\"%s\",", circularBuffer[i]);
		if (strlen(jsonBuffer) + strlen(lineBuffer) < bufferSize - 1)
			strcat(jsonBuffer, lineBuffer);
		i = (i + 1) % circBufferSize;
	} while (i != currentIndex);

	if (jsonBuffer[strlen(jsonBuffer) - 1] == ',')
		jsonBuffer[strlen(jsonBuffer) - 1] = '\0'; // Remove trailing comma

	strcat(jsonBuffer, "]");
	lastSentIndex = currentIndex; // Update lastSentIndex to the currentIndex
	return jsonBuffer;
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

		currentIndex = (currentIndex + 1) % circBufferSize;
		circularBuffer[currentIndex][0] = '\0';
		currentLine = nextLine + 1; // Move to the start of the next line
	}

	// Move to the next index if newLine is true or end of the line reached
	if (newLine) {
		currentIndex = (currentIndex + 1) % circBufferSize;
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

