#include <Arduino.h>
#include "WebServerInterface.h"
#include "util.h"

const int bufferSize    = 100;                       // Number of lines in the buffer
const int maxLineLength = 128;                       // Maximum length of each line, including the null terminator
char      circularBuffer[bufferSize][maxLineLength]; // Circular buffer to hold the lines
int       currentIndex  = 0;                         // Current index in the buffer
int       lastSentIndex = 0;


void      handleUserInput(char* userInput);
String    getLinesAsJson();
WebServer server(80);

void WebServerInterface::init() {

	// Initialize the buffer with empty strings
    for (int i = 0; i < bufferSize; i++) {
        circularBuffer[i][0] = '\0'; // Set the first character to null terminator
    }

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		serialPrint(".");
	}

	serialPrintln("");
	serialPrint("Connected to ");
	serialPrintln(ssid);
	serialPrint("IP address: ");
	serialPrintln(WiFi.localIP().toString().c_str());

	server.on("/", []() {
		server.send(200, "text/html", "<!doctypehtml><html lang=en><meta charset=UTF-8><title>Microcontroller Serial Interface</title><style>body{font-family:Arial,sans-serif;margin:0;padding:0}#inputField{width:100%;box-sizing:border-box}#output{height:300px;overflow-y:scroll;background:#f0f0f0;margin-top:10px;padding:5px}button{margin-top:5px}</style><input id=inputField placeholder=\"Type here...\"> <button onclick=submitData()>Submit</button> <button onclick=clearOutput()>Clear Output</button><div id=output></div><script>var e=document.getElementById(\"inputField\"),t=document.getElementById(\"output\"),n=[],o=0;e.addEventListener(\"keydown\",(function(t){var a;\"Enter\"===t.key?(a=e.value,n.push(a),o=n.length,fetch(\"/submit\",{method:\"POST\",body:a}),e.value=\"\"):\"ArrowUp\"===t.key?(o=Math.max(o-1,0),e.value=n[o]||\"\"):\"ArrowDown\"===t.key&&(o=Math.min(o+1,n.length-1),e.value=n[o]||\"\")})),setInterval((function(){fetch(\"/poll\").then((e=>e.json())).then((e=>{e.forEach((e=>{var n=document.createElement(\"div\");n.textContent=e,t.appendChild(n)}))}))}),2e3);</script>");
	});

	server.on("/submit", HTTP_POST, []() {
		if (server.hasArg("plain")) {

			char userData[1024];
			strncpy(userData, server.arg("plain").c_str(), 1023);
			userData[1023] = '\0';

			serialPrintln(userData); // Do something with postData
			handleUserInput(userData);
		}
	});

	server.on("/poll", []() {
		server.send(200, "application/json", getLinesAsJson());
	});

	//ElegantOTA.begin(&server);    // Start ElegantOTA
	server.begin();
	serialPrintln("HTTP server started");
}

void WebServerInterface::update() {
  server.handleClient();
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
        if (circularBuffer[i][0] != '\0') { // Check if the line is not empty
            json += "\"";
            json += String(circularBuffer[i]);
            json += "\",";
        }
        i = (i + 1) % bufferSize;
    } while (i != currentIndex);

    if (json.endsWith(",")) {
        json.remove(json.length() - 1); // Remove the trailing comma
    }
    
    json += "]";
    lastSentIndex = currentIndex; // Update lastSentIndex to the currentIndex
    return json;
}


void addToBuffer(const char* text) {
    // Ensure we don't copy more than the maxLineLength - 1 characters (leaving space for null terminator)
    strncpy(circularBuffer[currentIndex], text, maxLineLength - 1);
    // Manually add null terminator in case text was truncated
    circularBuffer[currentIndex][maxLineLength - 1] = '\0';
    // Increment and wrap index if necessary
    currentIndex = (currentIndex + 1) % bufferSize;
}

void serialPrint(const char* text)
{
	addToBuffer(text);
	Serial.println(text);
}

void serialPrintln(const char* line)
{
	addToBuffer(line);
	Serial.println(line);
}
