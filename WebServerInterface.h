#ifndef __WEBSERVER_INTERFACE__
#define __WEBSERVER_INTERFACE__

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
//#include <ElegantOTA.h>
//#include <elop.h>


class WebServerInterface
{
private:
	bool wifiRunning;
	bool serverRunning;

public:
	void listWifi();
	void printStatus();

	void startWifi();
	void stopWifi();

	void startServer();
	void stopServer();

	void update();
};

#endif
