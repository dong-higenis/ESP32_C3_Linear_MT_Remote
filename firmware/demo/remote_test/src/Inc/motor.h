#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

/**
* @brief 모터 상수
*/
#define MOTOR_MAX_SPEED 255
#define MOTOR_STOP_SPEED 0 
#define MOTOR_PWM_8BIT_RESOLUTION 8
#define MOTOR_PWM_FREQ 5000

void motorSetup(void);
void motorGoUp(void);
void motorGoDown(void);
void motorStop(void);
void motorSetLock(void);
void motorSetUnlock(void);
bool isMotorLocked(void);

#endif