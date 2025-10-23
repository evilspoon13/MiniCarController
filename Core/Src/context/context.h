#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "common.h"

// fwd declared
typedef enum _MotorCommand {
    CMD_STOP,
    CMD_FORWARD,
    CMD_BACKWARD,
    CMD_TURN_LEFT,
    CMD_TURN_RUGHT
} MotorCommand_e;

typedef enum _SystemState_e {
    STATE_INIT,
    STATE_RUNNING,
    STATE_SHUTDOWN,
    STATE_ERROR
} SystemState_e;

typedef struct _SystemState {
    SystemState_e state;
    MotorCommand_e motor_command;
    uint8_t speed;
    bool new_command_flag;
} SystemState;

#endif

