#include "Inc/button.h"

bool btn1_state = HIGH;
bool btn2_state = HIGH;

bool is_pressing = false;

btn_decoded_t last_btn_state = BTN_NONE;

void btnSetup(void)
{
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
}

// void btnTask(void)
// {
//   uint32_t press_duration = getAllButtonPressDuration();
  
//   if (press_duration >= PRESSED_TIME_FOR_DELETE_MODE && getRemoteModeState() == REMOTE_LEARN) 
//   {
//     remote_mode.mode = REMOTE_DELETE;
//     userWantsLearnRemote(false);
//     dont_record_press_time = false; // delete시에만 시계 작동시작
//   }

//   if (press_duration >= PRESSED_TIME_FOR_LEARN_MODE)
//   {
//     if (getRemoteModeState() == REMOTE_IDLE) 
//     {
//       userWantsLearnRemote(YES); 
//       // 시계 작동 안해도댐
//       Serial.println("리모컨 등록을 원하실 경우, 리모컨 버튼을 꾹 누르고 계십시오.");
//     }
//   } 
// }

uint32_t getButtonPressDuration(void)
{
  static uint32_t press_start_time = RESET_TIMER;
  btn_decoded_t physic_btn_state = getButtonState();

   // 버튼 눌림 시작 감지
  if (!is_pressing && physic_btn_state != BTN_NONE) 
  {
    press_start_time = millis();
    last_btn_state = physic_btn_state;
    is_pressing = true;
  }
  else if (last_btn_state == BTN_NONE && last_btn_state == physic_btn_state) // 애시당초 버튼 뗀 상태면 타이머 계속 초기화
  {
    press_start_time = millis();
    return 0;
  }

  else if (physic_btn_state == BTN_NONE) // 뗐으면 무조건 한번 반환 후 타이머 처음부터
  {
    if (is_pressing)
    {
      is_pressing = false;
      return millis() - press_start_time;
    }
  }
  else if (physic_btn_state == BTN_PHYSIC_BOTH_PRESSED) // 둘다 누르고있으면 계속 타이머 측정값 리턴
  {
    if (is_pressing)
    {
      return millis() - press_start_time;
    }
  }
}

btn_decoded_t getButtonState(void)
{
  btn1_state = digitalRead(BTN1_PIN);
  btn2_state = digitalRead(BTN2_PIN);

  if (btn1_state && btn2_state)
  {
    return BTN_NONE;
  }
  else if (!btn1_state && btn2_state)
  {
    return BTN_A_PRESSED;
  }
  else if (btn1_state && !btn2_state)
  {
    return BTN_B_PRESSED;
  }
  else if (!btn1_state && !btn2_state)
  {
    return BTN_PHYSIC_BOTH_PRESSED;
  }
}

bool IsAllBtnsPressing(void)
{
  btn_decoded_t result = getButtonState();

  if(result == BTN_PHYSIC_BOTH_PRESSED)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void unlockButtonFlag(void)
{
  is_pressing = false;
}