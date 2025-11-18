#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "common.h"
#include "context.h"


/*
           ON NUCLEO
PA0 → ENA  A0
PB0 → IN1  A3
PB1 → IN2  CN10 26
PA1 → ENB  A1
PB2 → IN3  CN10 22
PB3 → IN4  D3
GND → GND  

*/

// Pin definitions
#define MOTOR1_IN1_PIN    GPIO_PIN_0
#define MOTOR1_IN1_PORT   GPIOB
#define MOTOR1_IN2_PIN    GPIO_PIN_1
#define MOTOR1_IN2_PORT   GPIOB

#define MOTOR2_IN3_PIN    GPIO_PIN_2
#define MOTOR2_IN3_PORT   GPIOB
#define MOTOR2_IN4_PIN    GPIO_PIN_3
#define MOTOR2_IN4_PORT   GPIOB

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
