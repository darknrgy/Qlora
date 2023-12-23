#include "config.h"

Config::Config() {

}

void Config::toggleDebug() {
	debug = !debug;
}

bool Config::isDebug() {
	return debug;
}

