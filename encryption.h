#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <Arduino.h>
#include "config.h"


class Encryption {
public:
	static String encrypt(const String& msg, const String& seed);

private:
	Encryption() {}  // Private constructor to prevent instantiation
	static String xorCipher(const String& msg, const String& key, char replaceNull);
	static String shiftString(const String& str, int shiftAmount);
	static int hexStringToReducedNumber(const char* hexString);
};

#endif // ENCRYPTION_H
