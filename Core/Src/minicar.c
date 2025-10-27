#include "context.h"
#include "minicar.h"
#include "can.h"

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

    int status;
    if((status = ConfigCan(state)) != HAL_OK){
        state->cur_state = STATE_ERROR;
        return;
    }

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
}
