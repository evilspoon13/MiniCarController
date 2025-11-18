#include "minicar.h"
#include "can.h"
#include "context.h"
#include "motor.h"

void MinicarCanRx(SystemState* state, CAN_RxHeaderTypeDef rx_header, uint8_t rx_data[8])
{
    // process incoming CAN data

    uint32_t can_id = rx_header.StdId;
    switch(can_id)
    {
        case CANID_RX_HEARTBEAT:
            // process heartbeat
            state->last_rx_heartbeat = HAL_GetTick();
            break;
        case CANID_MOTOR_CMD:
            // parse motor command
            MotorCommand_e cmd = (MotorCommand_e)rx_data[0];
            uint8_t speed = rx_data[1];
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
    }

    MotorInit(state);

    state->active = true;
}

void MinicarShutdown(SystemState* state)
{
    state->active = false;
}

void MinicarIter(SystemState* state)
{
    static uint32_t last_tx = 0;
    uint32_t now = HAL_GetTick();

    
    // todo: use system state to flash led, not heartbeat

    if (state->motor.new_command_flag)
    {
        HandleMotorCommand(state);
        state->motor.new_command_flag = false;
    }

    // check heartbeat timeout
    if (state->last_rx_heartbeat < now - HEARTBEAT_TIMEOUT_MS)
    {
        // handle timeout
        // MotorsStopAll(state);
        state->active = false;
    }

    // transmit heartbeat to laptop node
    if (now - last_tx > 1000)
    {
        uint8_t test_data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44};

        int status;
        if((status = CanTransmit(state->hw.can, CANID_TX_HEARTBEAT, test_data)) != HAL_OK){
            // handle error
            //HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        }
        last_tx = now;
    }
}
