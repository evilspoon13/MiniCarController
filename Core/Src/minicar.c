#include "can.h"
#include "context.h"
#include "minicar.h"

void MinicarCanRx(SystemState* state, CAN_RxHeaderTypeDef rx_header, uint8_t rx_data[8])
{
    // process incoming CAN data

    uint32_t can_id = rx_header.StdId;
    switch(can_id)
    {
        case CANID_RX_HEARTBEAT:
            // process heartbeat
            state->last_rx_heartbeat = HAL_GetTick();
            state->received_heartbeat = true;
            break;
        case CANID_MOTOR_CMD:
            // parse motor command
            MotorCommand_e cmd = (MotorCommand_e)rx_data[0];
            Speed_e speed = (Speed_e)rx_data[1];
            state->motor.motor_command = cmd;
            state->motor.speed = speed;
            state->motor.new_command_flag = true;
            break;
        // handle other CAN IDs as needed

        default:
            // unknown CAN ID
            break;
    }
}

void MinicarInit(SystemState* state)
{

    // init car
    HAL_TIM_Base_Start(state->hw.timer);

    int status;
    if((status = ConfigCan(state)) != HAL_OK)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }
}

void MinicarShutdown(SystemState* state)
{
    // todo: more stuff?
    state->active = false;
}

int MinicarWakeup(SystemState* state)
{
    // todo: motor stuff
    state->active = true;
    return HAL_OK;
}

void MinicarIter(SystemState* state)
{
    // Check if we received a heartbeat
    if(state->received_heartbeat)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        state->received_heartbeat = false;
    }

    //TEST: Send a message every second
    static uint32_t last_tx = 0;
    uint32_t now = HAL_GetTick();
    
    if (now - last_tx > 1000) {
        uint8_t test_data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44};

        int status;
        if((status = CanTransmit(state->hw.can, CANID_RX_HEARTBEAT, test_data)) != HAL_OK){
            // handle error
        }
        last_tx = now;
    }

    // // todo - handle new motor commands
    // if(state->motor.new_command_flag){
    //     MotorCommand_e cmd = state->motor.motor_command;
    //     Speed_e speed = state->motor.speed;
    //     state->motor.new_command_flag = false;

    //     switch(cmd){
    //         case CMD_STOP:
    //             MotorStop(state);
    //             break;
    //         case CMD_FORWARD:
    //             MotorForward(state, speed);
    //             break;
    //         case CMD_BACKWARD:
    //             MotorBackward(state, speed);
    //             break;
    //         case CMD_TURN_LEFT:
    //             MotorTurnLeft(state, speed);
    //             break;
    //         case CMD_TURN_RIGHT:
    //             MotorTurnRight(state, speed);
    //             break;
    //         default:
    //             break;
    //     }
    // }
}
