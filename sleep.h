#ifndef SLEEP_H
#define SLEEP_H

#include "config.h"
#include "esp_sleep.h"
#include "ullmillis.h"
#include <LoRa.h>

#define WAKEUP_REASON_TIMER 1
#define WAKEUP_REASON_LORA 2
#define WAKEUP_REASON_SENSE 3

#define WAKE_TIME 30000

class Sleep{
public:
	Sleep(LoRaClass* lora);
	int wakeupReason();
	void extendAwake();
	void sleepIfShould();

private:
	unsigned long long sleepTime = ullmillis() + WAKE_TIME;
	LoRaClass* lora;
};

#endif