#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "common.h"

typedef enum _MotorCommand {
    CMD_STOP,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_TURN_LEFT,
    CMD_TURN_RUGHT
} MotorCommand_e;

typedef enum _Speed_e {
    SPEED_STOPPED,
    SPEED_LOW,
    SPEED_MEDIUM,
    SPEED_HIGH,
} Speed_e;

typedef struct _Hardware {
    TIM_HandleTypeDef* timer;

    CAN_HandleTypeDef* can;
    CAN_TxHeaderTypeDef can_tx_header;
    uint32_t can_tx_mailbox;
} Hardware;

typedef struct _SystemState {

    // hardware components
    Hardware hw;

    bool active;
    uint64_t last_rx_heartbeat;

    // motor stuff
    struct {
        MotorCommand_e motor_command;
        Speed_e speed;
        bool new_command_flag;
    } motor;

    bool received_heartbeat;
} SystemState;

#endif

