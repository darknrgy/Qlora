#ifndef __WEBSERVER_INTERFACE__
#define __WEBSERVER_INTERFACE__

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
//#include <ElegantOTA.h>
//#include <elop.h>


class WebServerInterface
{
public:
	void init();
	void update();
};

#endif
