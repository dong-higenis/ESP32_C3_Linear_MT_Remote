#include "Inc/rf.h"

uint32_t captured_rf_code = RF_NOT_AVAILABLE; 
bool is_captured = false;
bool rf_flag = false;

uint32_t up_btn_val = 0;
uint32_t down_btn_val = 0;
uint32_t lock_btn_val = 0;
uint32_t stop_btn_val = 0;

RCSwitch mySwitch = RCSwitch();
extern Preferences preferences;

void rfSetup(void)
{
  mySwitch.enableReceive(digitalPinToInterrupt(RF_DAT_PIN));
}

uint32_t rfTask(void)
{
  static uint32_t safety_term = 0;

  static uint32_t rx_data = 0;
  static uint32_t last_rx_data = 0;

  if (mySwitch.available()) 
  {
    rx_data = mySwitch.getReceivedValue();
    mySwitch.resetAvailable();
    is_captured = true;
    captured_rf_code = rx_data;
    
    uint8_t slot_num = findRemoteSlot(rx_data);
    if(slot_num != RF_UNKNOWN_REMOTE_CONTROLLER)
    {
      // Serial.printf("[RF] 슬롯 %d 리모컨 인식\n", slot_num);
      
      char namespace_name[20];
      sprintf(namespace_name, "remote_%d", slot_num);

      preferences.begin(namespace_name, true);

      up_btn_val = preferences.getUInt("up_btn", 0);
      down_btn_val = preferences.getUInt("down_btn", 0);
      lock_btn_val = preferences.getUInt("lock_btn", 0);
      stop_btn_val = preferences.getUInt("stop_btn", 0);
      
      preferences.end();
      
      if(millis() - safety_term >= REMOTE_BTN_DEBOUNCE_TERM || last_rx_data != rx_data )
      {
        safety_term = millis();

        if(isMotorLocked() == false)
        {
          if(rx_data == up_btn_val)
          {
            updateButtonState(BTN_A_PRESSED);
          }
          else if(rx_data == down_btn_val)
          {
            updateButtonState(BTN_B_PRESSED);
          }
          else if(rx_data == stop_btn_val)
          {
            updateButtonState(BTN_D_PRESSED);
          }
       }
      
        if(rx_data == lock_btn_val)
        {
          if (isMotorLocked() == true)
          {
            motorSetUnlock();
          }
          else 
          {
            motorSetLock();
          }
        }
      }
    }
    else
    {
      // Serial.println("[RF] 미등록 리모컨");
      return RF_UNKNOWN_REMOTE_CONTROLLER;
    }
  }
  last_rx_data = rx_data;

  return RF_NOT_AVAILABLE;
}

bool isCapturedRfSignal(void)
{
  return is_captured;
}

void goCaptureNextSignal(void)
{
  is_captured = false;
}

uint32_t getCapturedData(void)
{
  return captured_rf_code;
}
