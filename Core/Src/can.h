#ifndef _CAN_H_
#define _CAN_H_

#include "context.h"

#define CANID_MOTOR_CMD 0x100

#define CANID_EMERGENCY_STOP 0x101

#define CANID_CONFIG 0x102

#define CANID_RX_HEARTBEAT 0x103

#define CANID_TX_HEARTBEAT 0x104

int CanInit(CAN_HandleTypeDef* hcan);

int CanTransmit(CAN_HandleTypeDef* hcan, uint32_t id, uint8_t data[8]);

int CanTxHeartbeat(SystemState* state);

int ConfigCan(SystemState* state);

#endif  