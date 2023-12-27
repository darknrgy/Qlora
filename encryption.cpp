#include "encryption.h"
#include "key.h" // ignored by git

String Encryption::xorCipher(const String& data) {
	size_t keySize = strlen(key);  // Assuming key is a char array in Key.h
	String output = "";

	for (size_t i = 0; i < data.length(); ++i) {
		char encryptedChar = data[i] ^ key[i % keySize];
		output += encryptedChar;
	}

	return output;
}