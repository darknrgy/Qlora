#include "sleep.h"
#include "util.h"


#define INTERRUPT_PIN GPIO_NUM_13

Sleep::Sleep(LoRaClass* lora) {
	this->lora = lora;

    esp_sleep_enable_ext0_wakeup(INTERRUPT_PIN,1); //1 = High, 0 = Low
	serialPrintln("Setup ESP32 to wake on pin 3");
}

void Sleep::extendAwake() {
	sleepTime = ullmillis() + WAKE_TIME;
}

void Sleep::sleepIfShould() {
	if (!CONFIG.isRelay()) return;
	if (ullmillis() > sleepTime) {
		lora->receive();
		if (CONFIG.isDebug()) {
			serialPrintln("Going to sleep...");
			delay(5);
		}
		digitalWrite(LED_BUILTIN, LOW);
		esp_light_sleep_start();
		digitalWrite(LED_BUILTIN, HIGH);
		extendAwake();
		if (CONFIG.isDebug()) serialPrintln("WAKEUP");
	}
}


	