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
    state->cur_state = STATE_INIT;

    // init car
    HAL_TIM_Base_Start(state->hw.timer);

    // CAN is already fully configured in MX_CAN1_Init()
    // No need to call ConfigCan anymore!

    // SUCCESS blink
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

    // enter idle state: waiting for heartbeat
    state->cur_state = STATE_IDLE;
}

void MinicarShutdown(SystemState* state)
{
    // todo: more stuff?
    state->cur_state = STATE_IDLE;
}

int MinicarWakeup(SystemState* state)
{
    // todo: motor stuff
    state->cur_state = STATE_ACTIVE;
    return HAL_OK;
}

void MinicarIter(SystemState* state)
{

    // // Debug: blink LED slowly to show main loop is running
    // static uint32_t last_blink = 0;
    // if (HAL_GetTick() - last_blink > 500) {  // Blink every 500ms
    //     HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    //     last_blink = HAL_GetTick();
    // }


    // Check if we received a heartbeat
    if(state->received_heartbeat)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        state->received_heartbeat = false;
    }

    // TEST: Send a message every second
    static uint32_t last_tx = 0;
    uint32_t now = HAL_GetTick();
    
    if (now - last_tx > 1000) {
        uint8_t test_data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44};
        CanTransmit(state->hw.can, CANID_RX_HEARTBEAT, test_data);
        last_tx = now;
    }

    
    // // After CanTransmit call, check mailbox status
    // if (now - last_tx > 1000) {
    //     uint8_t test_data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44};
        
    //     int status = CanTransmit(state->hw.can, CANID_RX_HEARTBEAT, test_data);
        
    //     // Check error status
    //     uint32_t error = HAL_CAN_GetError(state->hw.can);
        
    //     if (status == HAL_OK && error == HAL_CAN_ERROR_NONE) {
    //         // Success - quick blink
    //         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    //         HAL_Delay(50);
    //         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    //     } else {
    //         // Error - stay on
    //         HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    //     }
        
    //     last_tx = now;
    // }


    
    // static uint32_t last_tx = 0;
    // uint32_t now = HAL_GetTick();
    
    // if (now - last_tx > 1000) {
    //     uint8_t test_data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44};
        
    //     int result = CanTransmit(state->hw.can, 0x103, test_data);
        
    //     if (result == HAL_OK) {
    //         // Transmission successful - blink LED
            
    //         // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    //         // HAL_Delay(50);
    //         // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            
    //     } else {
            
    //         // Transmission FAILED - rapid blinking
    //         for(int i = 0; i < 5; i++) {
    //             HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    //             HAL_Delay(100);
    //         }
                
    //     }
        
    //     last_tx = now;
    // }

    //
    // transition to requested state
    //
    if(state->cur_state != STATE_IDLE && state->req_state == STATE_SHUTDOWN)
    {
        MinicarShutdown(state);
        return;
    }
    else if(state->cur_state != STATE_ACTIVE && state->req_state == STATE_IDLE)
    {
        MinicarWakeup(state);
        return;
    }

    //
    // set requested states
    //
    uint64_t current_time = HAL_GetTick();
    uint64_t elapsed_since_heartbeat = current_time - state->last_rx_heartbeat;
    
    if(state->cur_state == STATE_IDLE && elapsed_since_heartbeat < HEARTBEAT_TIMEOUT_MS){
        // we're in idle and received a recent heartbeat -> enter active
        state->req_state = STATE_ACTIVE;
        return;
    }
    else if(state->cur_state == STATE_ACTIVE && elapsed_since_heartbeat > HEARTBEAT_TIMEOUT_MS)
    {
        // we havent heard from the laptop node in more than heartbeat timeout ms -> enter shutdown
        state->req_state = STATE_SHUTDOWN;
        return;
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
