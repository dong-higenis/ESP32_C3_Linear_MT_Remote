#include "Inc/motor.h"

bool is_motor_stoped = false;
bool is_motor_locked = false;

void motorSetup(void)
{
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);

  ledcAttach(MOTOR_IN1_PIN, MOTOR_PWM_FREQ, MOTOR_PWM_8BIT_RESOLUTION);
  ledcAttach(MOTOR_IN2_PIN, MOTOR_PWM_FREQ, MOTOR_PWM_8BIT_RESOLUTION);
}

void motorGoUp(void)
{
  if(is_motor_locked == UNLOCK)
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_MAX_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
    is_motor_stoped = false;
  }
}

void motorGoDown(void)
{
  if(is_motor_locked == UNLOCK)
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_MAX_SPEED);
    is_motor_stoped = false;
  }
}

void motorStop(void)
{
  if (!is_motor_stoped)
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
    is_motor_stoped = true;
  }
}

void motorSetLock(void)
{
  is_motor_locked = LOCK;
}

void motorSetUnlock(void)
{
  is_motor_locked = UNLOCK;
}

bool isMotorLocked(void)
{
  return is_motor_locked;
}