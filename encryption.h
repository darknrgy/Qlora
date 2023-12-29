#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <Arduino.h>
#include "config.h"


class Encryption {
public:
	static const char* encrypt(const char* msg, const char* seed);

private:
	Encryption() {}  // Private constructor to prevent instantiation

	static const size_t maxMessageSize = 256;
	static const size_t maxKeySize     = 256;


	static void xorCipher(const char* msg, const char* key, char replaceNull, char* output);
	static void shiftKey(char* key, int shiftAmount);
	static int  hexStringToReducedNumber(const char* hexString);
};

#endif // ENCRYPTION_H
