#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include "config.h"

float getBatteryVoltage(int pin);
float correctVoltage(float inputValue);
const char* getDeviceId();

#define SERIAL_LOG_FORMAT(BUFFER_SIZE, FORMAT, ...) \
    do { \
        char output[BUFFER_SIZE]; \
        snprintf(output, BUFFER_SIZE, FORMAT, __VA_ARGS__); \
        Serial.println(output); \
    } while (0)

#define SERIAL_DEBUG_FORMAT(BUFFER_SIZE, FORMAT, ...) \
    if (CONFIG.isDebug()) { \
        char output[BUFFER_SIZE]; \
        snprintf(output, BUFFER_SIZE, FORMAT, __VA_ARGS__); \
        Serial.println(output); \
    }

#endif