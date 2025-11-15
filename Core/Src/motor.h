#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "common.h"
#include "context.h"

// Pin definitions
#define MOTOR1_IN1_PIN    GPIO_PIN_4 //PB4 - D5
#define MOTOR1_IN1_PORT   GPIOB
#define MOTOR1_IN2_PIN    GPIO_PIN_5 //PB5 - D4
#define MOTOR1_IN2_PORT   GPIOB

#define MOTOR2_IN3_PIN    GPIO_PIN_6 //PA6 - D12
#define MOTOR2_IN3_PORT   GPIOA
#define MOTOR2_IN4_PIN    GPIO_PIN_7 //PA7 - D11
#define MOTOR2_IN4_PORT   GPIOA

#define MAX_PWM 65  // Maximum safe PWM duty cycle

void MotorInit(SystemState *state);
void LeftMotorForward(SystemState *state, uint8_t speed);
void LeftMotorBackward(SystemState *state, uint8_t speed);
void LeftMotorStop(SystemState *state);
void RightMotorForward(SystemState *state, uint8_t speed);
void RightMotorBackward(SystemState *state, uint8_t speed);
void RightMotorStop(SystemState *state);
void MotorsStopAll(SystemState *state);

void HandleMotorCommand(SystemState* state);

#endif
