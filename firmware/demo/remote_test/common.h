#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

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
  BTN_NONE,
  BTN_A_PRESSED,
  BTN_B_PRESSED,
  BTN_C_PRESSED,
  BTN_D_PRESSED,
  BTN_PHYSIC_BOTH_PRESSED
} btn_decoded_t;

#define MAX_REMOTE_CONTROLLER_NUM 20
#define MAX_BTN_NUM 4

#define BTN_CHATTERED_TIME 53 // 32

#define SUCCEED 100 

extern int remote_count;                // 현재 등록된 리모컨 개수

#endif