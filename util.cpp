#include "util.h"

float getBatteryVoltage(int pin) {
	float sensorValue = analogRead(pin);
	float voltage = ((float) sensorValue / 4095.0f) * 3.3;
	voltage = voltage * ((VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2);
	return correctVoltage(voltage);
}

// Voltage Divider Correction using table and interpolation
const int tableSize = 36;
float inputTable[36] = {0.0f, 0.14f, 0.65f, 0.96f, 1.59f, 2.06f, 2.56f, 3.03f, 3.51f, 3.98f, 4.47f, 4.97f, 5.48f, 5.95f, 6.42f, 6.92f, 7.41f, 7.88f, 8.38f, 8.85f, 9.35f, 9.81f, 10.3f, 10.76f, 11.24f, 11.75f, 12.23f, 12.74f, 13.25f, 13.79f, 14.4f, 15.04f, 15.76f, 16.59f, 17.44f, 18.39f};
float outputTable[36] = {0.0f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f, 7.5f, 8.0f, 8.5f, 9.0f, 9.5f, 10.0f, 10.5f, 11.0f, 11.5f, 12.0f, 12.5f, 13.0f, 13.5f, 14.0f, 14.5f, 15.0f, 15.5f, 16.0f, 16.5f, 17.0f, 17.5f, 18.0f};

float correctVoltage(float inputValue) {
	// Handle boundary conditions
	if (inputValue <= inputTable[0]) return outputTable[0];
	if (inputValue >= inputTable[tableSize - 1]) return outputTable[tableSize - 1];

	// Search for the closest index
	int i = 0;
	for (i = 0; i < tableSize - 1; i++) {
			if (inputTable[i] == inputValue) {
					return outputTable[i];
			} else if (inputTable[i+1] > inputValue) {
					break;
			}
	}

	// Linear interpolation
	float delta_x = inputTable[i+1] - inputTable[i];
	float delta_y = outputTable[i+1] - outputTable[i];
	float slope = delta_y / delta_x;
	return outputTable[i] + slope * (inputValue - inputTable[i]);
}

String getDeviceId() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);  // Retrieve the MAC address

    // Convert MAC address to a hex string
    String macStr;
    for (int i = 0; i < 6; ++i) {
        if (mac[i] < 16) macStr += "0";  // Add leading zero for single digit hex values
        macStr += String(mac[i], HEX);
    }

    // Ensure the MAC string is in the correct format
    macStr.toUpperCase();

    // Extract and return the last 8 characters
    return macStr.substring(macStr.length() - 8);
}