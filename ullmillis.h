#ifndef ULLMILLS_H
#define ULLMILLS_H

inline unsigned long long ullmillis() {
    static unsigned long long offset = 0;
    static unsigned long previousMillis = millis();

    unsigned long currentMillis = millis();
    if (currentMillis < previousMillis) {
        offset += 4294967296ULL; // Corrected the offset value
    }
    previousMillis = currentMillis; // Update previousMillis for the next call

    return currentMillis + offset;
}

#endif // ULLMILLS_H