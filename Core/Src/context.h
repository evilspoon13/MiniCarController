#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "common.h"

typedef enum _MotorCommand {
    CMD_STOP,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_TURN_LEFT,
    CMD_TURN_RIGHT
} MotorCommand_e;

typedef enum _SafetyState {
    SAFETY_NORMAL,           // Normal operation, heartbeat OK
    SAFETY_DEGRADED,         // Heartbeat warning (approaching timeout)
    SAFETY_EMERGENCY_STOP,   // Heartbeat lost, motors stopped
    SAFETY_FAULT             // Critical fault condition
} SafetyState_e;

typedef struct _Hardware {
    TIM_HandleTypeDef* timer;
    CAN_HandleTypeDef* can;
    CAN_TxHeaderTypeDef can_tx_header;
    uint32_t can_tx_mailbox;
} Hardware;

typedef struct _SystemState {
    Hardware hw;
    bool active;
    uint64_t last_rx_heartbeat;
    uint64_t last_tx_heartbeat;
    struct {
        MotorCommand_e motor_command;
        uint8_t speed;
        bool new_command_flag;
    } motor;
    struct {
        SafetyState_e state;
        uint32_t heartbeat_loss_count;
        uint32_t last_state_change;
        bool emergency_stop_triggered;
    } safety;
} SystemState;

#endif

