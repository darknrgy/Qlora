#include "awake.h"
#include "config.h"
#include "esp_sleep.h"

Awake::Awake() {
	init();
}

void Awake::init() {
	esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
	esp_sleep_enable_ext0_wakeup(INTERRUPT_PIN,1);
}

	