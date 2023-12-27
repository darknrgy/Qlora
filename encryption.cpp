#include "encryption.h"
#include "key.h"

String Encryption::encrypt(const String& msg, const String& seed) {
	if (encryptionKey.isEmpty()) return msg;

	String seededEncryptedKey = xorCipher(encryptionKey, seed);
	String encryptedMsg = xorCipher(msg, seededEncryptedKey);
	return encryptedMsg;

}

String Encryption::xorCipher(const String& msg, const String& key) {
	size_t keySize = key.length();
	String output = "";

	for (size_t i = 0; i < msg.length(); ++i) {
		char encryptedChar = msg[i] ^ key[i % keySize];
		output += encryptedChar;
	}

	return output;
}
