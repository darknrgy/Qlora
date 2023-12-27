#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <Arduino.h>

class Encryption {
public:
	static String xorCipher(const String& data);

private:
	Encryption() {}  // Private constructor to prevent instantiation
};

#endif // ENCRYPTION_H
