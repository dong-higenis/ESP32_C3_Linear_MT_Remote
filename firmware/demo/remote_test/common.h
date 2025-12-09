#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEClient.h>

/**
* @brief Pin 번호
*/
#define LED_PIN 2
#define LED2_PIN 1
#define BTN1_PIN 0
#define BTN2_PIN 4
#define MOTOR_CURRENT_ADC_PIN 3
#define MOTOR_IN1_PIN 18
#define MOTOR_IN2_PIN 19
#define RF_DAT_PIN 10

typedef enum
{
  RF_NOT_AVAILABLE,
  RF_ALREADY_EXIST,
  RF_NO_REMOTE_CONTROLLER_IN_MEMORY,
  RF_MEMORY_FULL,
  RF_UNKNOWN_REMOTE_CONTROLLER = 0xFF

} common_error_t;

typedef enum
{
  RF_RESET_BUFFER
} result_t;

typedef enum
{
  REMOTE_IDLE,
  REMOTE_LEARN,
  REMOTE_CAPTURED,
  REMOTE_DONE,
  REMOTE_DELETE
} remote_state_t;

typedef enum 
{
  BTN_NONE,
  BTN_A_PRESSED,
  BTN_B_PRESSED,
  BTN_C_PRESSED,
  BTN_D_PRESSED,
  BTN_PHYSIC_BOTH_PRESSED
} btn_decoded_t;

#define UNLOCK false
#define LOCK true

#define RESET_TIMER 0

#define MAX_REMOTE_CONTROLLER_NUM 20
#define MAX_BTN_NUM 4

#define BTN_CHATTERED_TIME 53 // 32

#define SUCCEED 100 

extern int remote_count;                // 현재 등록된 리모컨 개수

remote_state_t getRemoteModeState(void);
void setRemoteModeState(remote_state_t state);

void setCapturedRfData(uint32_t data);
void updateButtonState(btn_decoded_t state);

#endif