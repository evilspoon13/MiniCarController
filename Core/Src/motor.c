#include "motor.h"

void MotorInit(SystemState* state)
{
    HAL_TIM_PWM_Start(state->hw.timer, TIM_CHANNEL_1); //ena
    HAL_TIM_PWM_Start(state->hw.timer, TIM_CHANNEL_2); //enb

    HAL_GPIO_WritePin(MOTOR1_IN1_PORT, MOTOR1_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR1_IN2_PORT, MOTOR1_IN2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR2_IN3_PORT, MOTOR2_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR2_IN4_PORT, MOTOR2_IN4_PIN, GPIO_PIN_RESET);
    
    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_2, 0);
}

void LeftMotorForward(SystemState* state, uint8_t speed)
{
    if(speed > MAX_PWM)
    {
        speed = MAX_PWM;
    }

    HAL_GPIO_WritePin(MOTOR1_IN1_PORT, MOTOR1_IN1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR1_IN2_PORT, MOTOR1_IN2_PIN, GPIO_PIN_RESET);

    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_1, speed);
}

void LeftMotorBackward(SystemState* state, uint8_t speed)
{
    if(speed > MAX_PWM)
    {
        speed = MAX_PWM;
    }

    HAL_GPIO_WritePin(MOTOR1_IN1_PORT, MOTOR1_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR1_IN2_PORT, MOTOR1_IN2_PIN, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_1, speed);
}

void LeftMotorStop(SystemState* state)
{
    HAL_GPIO_WritePin(MOTOR1_IN1_PORT, MOTOR1_IN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR1_IN2_PORT, MOTOR1_IN2_PIN, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_1, 0);
}

void RightMotorForward(SystemState* state, uint8_t speed)
{
    if(speed > MAX_PWM)
    {
        speed = MAX_PWM;
    }

    HAL_GPIO_WritePin(MOTOR2_IN3_PORT, MOTOR2_IN3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR2_IN4_PORT, MOTOR2_IN4_PIN, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_2, speed);
}

void RightMotorBackward(SystemState* state, uint8_t speed)
{
    if(speed > MAX_PWM)
    {
        speed = MAX_PWM;
    }

    HAL_GPIO_WritePin(MOTOR2_IN3_PORT, MOTOR2_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR2_IN4_PORT, MOTOR2_IN4_PIN, GPIO_PIN_SET);
    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_2, speed);
}

void RightMotorStop(SystemState* state)
{
    HAL_GPIO_WritePin(MOTOR2_IN3_PORT, MOTOR2_IN3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR2_IN4_PORT, MOTOR2_IN4_PIN, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(state->hw.timer, TIM_CHANNEL_2, 0);
}

void MotorsStopAll(SystemState* state)
{
    LeftMotorStop(state);
    RightMotorStop(state);
}

void HandleMotorCommand(SystemState* state)
{
    if (state->motor.new_command_flag)
    {
        MotorCommand_e cmd = state->motor.motor_command;
        uint8_t speed = state->motor.speed;
        state->motor.new_command_flag = false;

        switch(cmd){
            case CMD_STOP:
                MotorsStopAll(state);
                break;
            case CMD_FORWARD:
                LeftMotorForward(state, speed);
                RightMotorForward(state, speed);
                break;
            case CMD_BACKWARD:
                LeftMotorBackward(state, speed);
                RightMotorBackward(state, speed);
                break;
            case CMD_TURN_LEFT:
                LeftMotorBackward(state, speed);
                RightMotorForward(state, speed);
                break;
                
            case CMD_TURN_RIGHT:
                LeftMotorForward(state, speed);
                RightMotorBackward(state, speed);
                break;
            default:
                break;
        }
    }
}