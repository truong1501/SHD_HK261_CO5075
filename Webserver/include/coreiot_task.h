#pragma once
#include "global.h"

// True while the MQTT session with CoreIOT is up (reported on the dashboard)
extern volatile bool isCoreIOTConnected;

void coreiot_task(void *pvParameters);
