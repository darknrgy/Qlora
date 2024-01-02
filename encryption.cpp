#include "encryption.h"
#include "key.h"
#include "util.h"


const char* Encryption::encrypt(const char* msg, const char* seed) {
    static char encryptedMsg[maxMessageSize];

    // If we have an empty key don't use any encryption
    if (strlen(encryptionKey) == 0) {
        strcpy(encryptedMsg, msg);
        return encryptedMsg;
    }

    // Otherwise shift the string and use an xorCypher
    char keyCopy[maxKeySize];
    strcpy(keyCopy, encryptionKey);

    for (int i = 0; keyCopy[i] != '\0'; i++) {
        keyCopy[i] += 127;
    }

    int shiftAmount = hexStringToReducedNumber(seed);
    shiftKey(keyCopy, shiftAmount);

    xorCipher(msg, keyCopy, 0x00, encryptedMsg);
    return encryptedMsg;
}


void Encryption::xorCipher(const char* msg, const char* key, char replaceNull, char* output) {
    size_t keySize = strlen(key);
    size_t msgLength = strlen(msg);

    for (size_t i = 0; i < msgLength; ++i) {
        char encryptedChar = msg[i] ^ key[i % keySize];
        if (replaceNull == 0x00 && encryptedChar == 0x00) {
            serialPrintln("0x00 FOUND!");
        }
        if (encryptedChar == 0x00) encryptedChar = replaceNull;
        output[i] = encryptedChar;
    }
    output[msgLength] = '\0'; // Ensure null termination
}


void Encryption::shiftKey(char* key, int shiftAmount) {
    int len = strlen(key);
    char temp[maxKeySize];

    for (int i = 0; i < len; ++i) {
        int newIndex = (i + shiftAmount) % len;
        temp[i] = key[newIndex];
    }
    temp[len] = '\0';

    strcpy(key, temp);
}

int Encryption::hexStringToReducedNumber(const char* hexString) {
    unsigned long hexNumber = strtoul(hexString, NULL, 16);  // Convert hex string to number
    return hexNumber % 256;  // Reduce to a number between 0 and 255
}
