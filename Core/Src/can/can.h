#ifndef _CAN_H_
#define _CAN_H_

#include "common.h"
#include "context.h"

#define CANID_MOTOR_CMD 0x100
#define CANID_EMERGENCY_STOP 0x200
#define CANID_CONFIG 0x300

void CanRx(SystemState* system_state);

#endif