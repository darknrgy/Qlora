#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include "config.h"

float getBatteryVoltage(int pin);
float correctVoltage(float inputValue);
String getDeviceId();

#endif