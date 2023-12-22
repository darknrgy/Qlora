#ifndef CONFIG_H
#define CONFIG_H

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10  /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_STAY_AWAKE_RELAY 300
#define TIME_TO_STAY_AWAKE_WAIT_RELAY_MS 10000
#define INTERRUPT_PIN GPIO_NUM_25

// LoRa Pins
#define LORA_NSS 5
#define LORA_RST 14
#define LORA_DIO0 13

// Voltage Divider
#define VOLTAGE_READ_PIN0 A0
#define VOLTAGE_READ_PIN1 A1
#define R1 212500.0
#define R2 225000.0;

class Config {
public:
	static Config& getInstance() {
		static Config instance;
		return instance;
	}

	void toggleDebug();

	bool isDebug();

	Config(const Config&) = delete;
    void operator=(const Config&) = delete;

private:
	bool debug = true;
	Config();
};

#endif // CONFIG_H