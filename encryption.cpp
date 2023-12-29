#include "encryption.h"
#include "key.h"

String Encryption::encrypt(const String& msg, const String& seed) {
	if (encryptionKey.isEmpty()) return msg;

	String keyCopy = String(encryptionKey);

	for (int i = 0; i < keyCopy.length(); i++) {
		keyCopy.setCharAt(i, keyCopy.charAt(i) + 127);
	}

	int shiftAmount = hexStringToReducedNumber(seed.c_str());
	keyCopy = shiftString(keyCopy, shiftAmount);

	String encryptedMsg = xorCipher(msg, keyCopy, 0x00);
	return encryptedMsg;

}

String Encryption::xorCipher(const String& msg, const String& key, char replaceNull) {
	size_t keySize = key.length();
	String output = "";

	for (size_t i = 0; i < msg.length(); ++i) {
		char encryptedChar = msg[i] ^ key[i % keySize];
		if (replaceNull == 0x00 && encryptedChar == 0x00) {
			Serial.println("0x00 FOUND!");
		}
		if (encryptedChar == 0x00) encryptedChar = replaceNull;
		output += encryptedChar;
	}

	return output;
}

String Encryption::shiftString(const String& str, int shiftAmount) {
    String shiftedString = "";
    int len = str.length();

    for (int i = 0; i < len; ++i) {
        int newIndex = (i + shiftAmount) % len;  // Calculate new index with wrapping
        shiftedString += str[newIndex];  // Append the character at the new index
    }

    return shiftedString;  // Return the shifted string
}

int Encryption::hexStringToReducedNumber(const char* hexString) {
    unsigned long hexNumber = strtoul(hexString, NULL, 16);  // Convert hex string to number
    return hexNumber % 256;  // Reduce to a number between 0 and 255
}
