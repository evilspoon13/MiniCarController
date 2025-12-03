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
            // process heartbeat - update timestamp for safety monitoring
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

    // Initialize safety state
    state->safety.state = SAFETY_NORMAL;
    state->safety.heartbeat_loss_count = 0;
    state->safety.last_state_change = HAL_GetTick();
    state->safety.emergency_stop_triggered = false;
    state->last_rx_heartbeat = HAL_GetTick();

    state->active = true;
}

void MinicarEmergencyStop(SystemState* state)
{
    // Immediate motor shutdown
    MotorsStopAll(state);
    
    // Update safety state
    state->safety.emergency_stop_triggered = true;
    state->safety.heartbeat_loss_count++;
    state->safety.last_state_change = HAL_GetTick();
    
    // Clear any pending motor commands
    state->motor.new_command_flag = false;
    state->motor.motor_command = CMD_STOP;
    state->motor.speed = 0;
}

void MinicarSafetyCheck(SystemState* state)
{
    uint32_t now = HAL_GetTick();
    uint32_t time_since_heartbeat = now - state->last_rx_heartbeat;
    SafetyState_e prev_state = state->safety.state;
    
    // State machine for safety management
    switch(state->safety.state)
    {
        case SAFETY_NORMAL:
            if (time_since_heartbeat >= HEARTBEAT_TIMEOUT_MS)
            {
                // Critical: heartbeat timeout - emergency stop
                state->safety.state = SAFETY_EMERGENCY_STOP;
                state->safety.last_state_change = now;
                MinicarEmergencyStop(state);
            }
            else if (time_since_heartbeat >= HEARTBEAT_WARNING_MS)
            {
                // Warning: approaching timeout
                state->safety.state = SAFETY_DEGRADED;
                state->safety.last_state_change = now;
            }
            break;
            
        case SAFETY_DEGRADED:
            if (time_since_heartbeat >= HEARTBEAT_TIMEOUT_MS)
            {
                // Timeout reached - emergency stop
                state->safety.state = SAFETY_EMERGENCY_STOP;
                state->safety.last_state_change = now;
                MinicarEmergencyStop(state);
            }
            else if (time_since_heartbeat < HEARTBEAT_WARNING_MS)
            {
                // Recovered - return to normal
                state->safety.state = SAFETY_NORMAL;
                state->safety.last_state_change = now;
            }
            break;
            
        case SAFETY_EMERGENCY_STOP:
            // Stay in emergency stop until heartbeat recovers
            if (time_since_heartbeat < HEARTBEAT_WARNING_MS)
            {
                // Heartbeat recovered - return to normal
                state->safety.state = SAFETY_NORMAL;
                state->safety.last_state_change = now;
                state->safety.emergency_stop_triggered = false;
            }
            else
            {
                // Ensure motors stay stopped
                MotorsStopAll(state);
            }
            break;
            
        case SAFETY_FAULT:
            // Fault state - requires manual recovery
            MotorsStopAll(state);
            state->active = false;
            break;
    }
}

void MinicarUpdateSafetyLED(SystemState* state)
{
    static uint32_t last_led_toggle = 0;
    uint32_t now = HAL_GetTick();
    uint32_t blink_period = 0;
    
    // LED pattern based on safety state
    switch(state->safety.state)
    {
        case SAFETY_NORMAL:
            // Slow blink - all good
            blink_period = SAFETY_LED_SLOW_BLINK_MS;
            break;
            
        case SAFETY_DEGRADED:
            // Medium blink - warning
            blink_period = SAFETY_LED_SLOW_BLINK_MS / 2;
            break;
            
        case SAFETY_EMERGENCY_STOP:
            // Fast blink - emergency
            blink_period = SAFETY_LED_FAST_BLINK_MS;
            break;
            
        case SAFETY_FAULT:
            // Solid on - fault
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
            return;
    }
    
    if (now - last_led_toggle >= blink_period)
    {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        last_led_toggle = now;
    }
}

void MinicarIter(SystemState* state)
{
    static uint32_t last_tx = 0;
    uint32_t now = HAL_GetTick();

    // CRITICAL: Safety check first - monitors heartbeat and manages safety state
    MinicarSafetyCheck(state);
    
    // Update LED to reflect safety state
    MinicarUpdateSafetyLED(state);

    // Only process motor commands if in safe state
    if (state->motor.new_command_flag && 
        state->safety.state != SAFETY_EMERGENCY_STOP &&
        state->safety.state != SAFETY_FAULT)
    {
        HandleMotorCommand(state);
        state->motor.new_command_flag = false;
    }
    else if (state->motor.new_command_flag && 
             (state->safety.state == SAFETY_EMERGENCY_STOP || 
              state->safety.state == SAFETY_FAULT))
    {
        // Reject commands in emergency/fault state
        state->motor.new_command_flag = false;
    }

    // Transmit heartbeat to laptop node with safety state
    if (now - last_tx > 1000)
    {
        uint8_t status_data[8] = {
            0xAA,                           // Header
            (uint8_t)state->safety.state,   // Safety state
            state->safety.emergency_stop_triggered ? 0xFF : 0x00,
            (uint8_t)(state->safety.heartbeat_loss_count & 0xFF),
            0x00, 0x00, 0x00, 0x00
        };

        int status;
        if((status = CanTransmit(state->hw.can, CANID_TX_HEARTBEAT, status_data)) != HAL_OK){
            // CAN transmission error - potential fault condition
            if (state->safety.state != SAFETY_FAULT)
            {
                state->safety.state = SAFETY_FAULT;
                MinicarEmergencyStop(state);
            }
        }
        last_tx = now;
    }
}
