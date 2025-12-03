#ifndef _MINICAR_H_
#define _MINICAR_H_

#include "common.h"
#include "context.h"
#include "can.h"
#include "motor.h"

#define HEARTBEAT_TIMEOUT_MS 5000
#define HEARTBEAT_WARNING_MS 3000  // Warning threshold before timeout
#define SAFETY_LED_FAST_BLINK_MS 100
#define SAFETY_LED_SLOW_BLINK_MS 500

void MinicarInit(SystemState* state);
void MinicarIter(SystemState* state);
void MinicarEmergencyStop(SystemState* state);
void MinicarSafetyCheck(SystemState* state);
void MinicarUpdateSafetyLED(SystemState* state);

#endif
