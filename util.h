#ifndef UTIL_H
#define UTIL_H

#include <Arduino.h>
#include "config.h"
#include <LoRa.h>

float getBatteryVoltage(int pin);
float correctVoltage(float inputValue);
const char* getDeviceId();

#define SERIAL_LOG_FORMAT(BUFFER_SIZE, FORMAT, ...) \
    do { \
        char output[BUFFER_SIZE]; \
        snprintf(output, BUFFER_SIZE, FORMAT, __VA_ARGS__); \
        serialPrintln(output); \
    } while (0)

#define SERIAL_DEBUG_FORMAT(BUFFER_SIZE, FORMAT, ...) \
    if (CONFIG.isDebug()) { \
        char output[BUFFER_SIZE]; \
        snprintf(output, BUFFER_SIZE, FORMAT, __VA_ARGS__); \
        serialPrintln(output); \
    }

// Helper function to replace newline with space
inline void replaceNewlineWithSpace(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            str[i] = ' ';
        }
    }
}

void serialPrint(const char* text);
void serialPrintln(const char* line);


#endif