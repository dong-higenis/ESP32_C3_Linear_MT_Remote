
#include <RCSwitch.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEClient.h>

#include "common.h"
#include "remote_controller.h"
#include "remote_wifi.h"

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

/**
* @brief 모터 관련
*/
#define MOTOR_GAIN 0.554f
#define MOTOR_MIN_SPEED 1000
#define MOTOR_STOP_SPEED 0 

#define FAST_LED 100

#define LEARN_SAMPLE_MAX 50
#define MIN_SAMPLE_DATA_NUM 10

#define REMOTE_LEARN_MODE_INTERVEL 500
#define REMOTE_LEARN_RECOGNIZED_INTERVEL 100

#define LOW_ADC_TIMEOUT 3000
#define LEARN_MODE_TIMEOUT 10000

#define LEARN_SUCCEED_CATURE_TERM 5000
#define LEARN_COMPLETE_TERM 3000

#define PRESSED_TIME_FOR_DELETE_MODE 10000
#define PRESSED_TIME_FOR_LEARN_MODE 3000

#define GO_IDLE_AFTER_3000MS 3000

#define DO_DELETE_AFTER_2000MS 2000

#define REMOTE_BTN_DEBOUNCE_TERM 70

#define YES true
#define NO false
#define OK true
#define UNLOCK false
#define LOCK true

#define RESET_TIMER 0

typedef enum
{
  REMOTE_IDLE,
  REMOTE_LEARN,
  REMOTE_CAPTURED,
  REMOTE_DONE,
  REMOTE_DELETE
} remote_state_t;

typedef struct 
{
  remote_state_t mode;
  bool btn_flag;
  bool got_captured_flag;
  uint32_t up_btn_val;
  uint32_t down_btn_val;
  uint32_t lock_btn_val;
  uint32_t stop_btn_val;
  bool lock_flag;
  bool motor_stoped;
  btn_decoded_t last_btn_state;

  uint32_t first_learn_signal_data;
  uint8_t  learn_samples[LEARN_SAMPLE_MAX];
  uint8_t  learn_sample_index;

  uint32_t btn_timer;

} remote_mode_t;

remote_mode_t remote_mode = {REMOTE_IDLE, false, false, 0, 0, 0, 0, false, false, BTN_NONE, BTN_NONE, 0, {0}, 0, 0};

uint32_t captured_rf_code = RF_NOT_AVAILABLE; 

bool btn1_state = HIGH;
bool btn2_state = HIGH;

float motor_current = 0.0f;

bool btn_interrupt_flag_f = false;
bool btn_interrupt_flag_s = false;

RCSwitch mySwitch = RCSwitch();

extern Preferences preferences; // cpp에서만 생성가능해서, extern함

void setup() 
{
  serialSetup(115200);
  ledSetup();
  btnSetup();
  adcSetup();
  motorSetup();
  rfSetup();
  remoteSetup();
  wifiSetup();
}

void loop() 
{
  btnTask();
  adcTask();
  physicBtnTask();
  modeUpdate();
  rfTask();
  cliTask();
  handleWiFiRequests();
}

void modeUpdate(void)
{
  static uint8_t result = 0;
  static uint32_t timeout_timer = RESET_TIMER;
  static uint32_t succeed_timer = RESET_TIMER;
  static uint32_t done_timer = RESET_TIMER;
  
  switch(remote_mode.mode)
  {
    case REMOTE_IDLE:
      captured_rf_code = RF_NOT_AVAILABLE;
      
      allLedOff();
      
      if(remote_mode.btn_flag)
      {
        remote_mode.mode = REMOTE_LEARN;
        timeout_timer = millis(); // 시계 시작
      }
      break;
    
    case REMOTE_LEARN:
      ledTask(REMOTE_LEARN_MODE_INTERVEL);

      if (remote_mode.got_captured_flag)
      {
        pushLearnSample(); // 데이터가 들어올 때 마다, 배열에 데이터 삽입
        remote_mode.got_captured_flag = false;
        
        if (millis () - timeout_timer >= LEARN_SUCCEED_CATURE_TERM) // 시계 여기서 체크
        {
          if (isLearnDataStable())
          {
            remote_mode.mode = REMOTE_CAPTURED;
          }
          else
          {
            Serial.println("[LEARN] 신호 에러 - 재시도 필요");
            remote_mode.mode = REMOTE_IDLE;
          }
          resetLearnSamples();
          succeed_timer = millis(); // 다음 시계 시작
        }
      }
      if (millis() - timeout_timer >= LEARN_MODE_TIMEOUT)
      {
        remote_mode.mode = REMOTE_IDLE;  
      }
      break;
    
    case REMOTE_CAPTURED: // 여기서 완료되면 빠르게 깜빡
      ledTask(REMOTE_LEARN_RECOGNIZED_INTERVEL);
      
      motorSetLock(true); // 이미 등록된 모터가 움직이지 않도록 lock을 건다.

      if(millis() - succeed_timer >= LEARN_COMPLETE_TERM)
      {
        result = learnNewRemote(captured_rf_code);
        
        if(result == SUCCEED)
       {
         Serial.println("[MODE] 저장 완료!");
         captured_rf_code = RF_RESET_BUFFER;

         motorSetLock(false);

         remote_mode.mode = REMOTE_DONE;
       }
       else if(result == RF_ALREADY_EXIST)
       {
         Serial.println("[MODE] 이미 등록됨");
         captured_rf_code = RF_RESET_BUFFER;

         motorSetLock(false);

         remote_mode.mode = REMOTE_IDLE;
       }
      }
      break;

    case REMOTE_DONE:
      allLedOff();
      
      if(done_timer == RESET_TIMER) 
      {
        done_timer = millis();
      }
      
      if(millis() - done_timer > GO_IDLE_AFTER_3000MS)
      {
        // Serial.println("[MODE] IDLE 복귀");
        remote_mode.mode = REMOTE_IDLE;
        motorSetLock(false);
        done_timer = RESET_TIMER;
      }
      break;

    case REMOTE_DELETE:
    {
      static uint32_t delete_start_time = RESET_TIMER;
  
      // 처음 진입 시 타이머 초기화
      if (delete_start_time == RESET_TIMER)
      {
        delete_start_time = millis();
        motorStop();
        motorSetLock(true);
      }

      ledTask(FAST_LED);
  
      if (millis() - delete_start_time >= DO_DELETE_AFTER_2000MS)
      {
        deleteAllRemotes();
        Serial.println("[MODE] 공장 초기화 완료");

        allLedOff();

        delete_start_time = RESET_TIMER;
        remote_mode.mode = REMOTE_DONE;
      }
    }  
    break;

  default :
    break;
  }
}

void serialSetup(uint32_t baud)
{
  Serial.begin(baud);
}

void ledSetup(void)
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  allLedOff();
}

void ledTask(uint32_t led_task_intervel)
{
  static uint32_t led_timer = RESET_TIMER;
  static bool led_state = HIGH;
  
  if(millis() - led_timer >= led_task_intervel)
  {
    led_timer = millis();
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    digitalWrite(LED2_PIN, led_state);
  }
}

void btnSetup(void)
{
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
}

void btnTask(void)
{
  static uint32_t press_start_time = RESET_TIMER;
  static bool dont_record_press_time = NO;

  bool all_btn_pressing = IsAllBtnsPressing();

  if (all_btn_pressing == YES && dont_record_press_time == NO) 
  {
    press_start_time = millis();
    dont_record_press_time = OK;
  }
  else if (all_btn_pressing == NO)
  {
    userWantsLearnRemote(NO);
    dont_record_press_time = NO;
    return;
  }

  uint32_t press_duration = millis() - press_start_time;
  
  if (press_duration >= PRESSED_TIME_FOR_DELETE_MODE && remote_mode.mode == REMOTE_LEARN) 
  {
    remote_mode.mode = REMOTE_DELETE;
    userWantsLearnRemote(NO);
    dont_record_press_time = NO; // delete시에만 시계 작동시작
  }

  if (press_duration >= PRESSED_TIME_FOR_LEARN_MODE)
  {
    if (remote_mode.mode == REMOTE_IDLE) 
    {
      userWantsLearnRemote(YES); 
      // 시계 작동 안해도댐
      Serial.println("리모컨 등록을 원하실 경우, 리모컨 버튼을 꾹 누르고 계십시오.");
    }
  } 
}

void adcSetup(void)
{
  analogReadResolution(12);
}

void adcTask(void)
{
  static uint32_t raw_adc = 0;
  static float motor_voltage = 0;
  
  raw_adc = analogRead(MOTOR_CURRENT_ADC_PIN);
  motor_voltage = (raw_adc / 4095.0) * 3.3;
  float raw_current = motor_voltage / MOTOR_GAIN;
  
  static float filtered_current = 0;
  filtered_current = filtered_current * 0.9 + raw_current * 0.1;
  
  motor_current = filtered_current;
  
  checkCurrentTimeout(motor_current);
}

void motorSetup(void)
{
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);

  ledcAttach(MOTOR_IN1_PIN, 5000, 8);
  ledcAttach(MOTOR_IN2_PIN, 5000, 8);
}

void physicBtnTask(void)
{
  static bool btn_a_was_pressed = NO;
  static bool btn_b_was_pressed = NO;
    
  btn_decoded_t state = getButtonState();

  if (state == BTN_A_PRESSED) 
  {
    btn_a_was_pressed = YES;
    return;
  }
  else if (state == BTN_B_PRESSED)
  {
    btn_b_was_pressed = YES;
    return;
  }
  else if (state == BTN_PHYSIC_BOTH_PRESSED)
  {
    btn_a_was_pressed = YES;
    btn_b_was_pressed = YES;
    return;
  }
  
  if (!btn_a_was_pressed && !btn_b_was_pressed) 
  {
    return;
  }

  switch(state)
  {
    case BTN_A_PRESSED:
      updateButtonState(BTN_A_PRESSED);
      break;

    case BTN_B_PRESSED:
      updateButtonState(BTN_B_PRESSED);
      break;

    case BTN_NONE:
    case BTN_PHYSIC_BOTH_PRESSED:
    default:
      break;
  }

  if (btn_a_was_pressed)
  {
    updateButtonState(BTN_A_PRESSED);
    btn_a_was_pressed = NO;
  }

  if (btn_b_was_pressed)
  {
    updateButtonState(BTN_B_PRESSED);
    btn_b_was_pressed = NO;
  }
}

void rfSetup(void)
{
  mySwitch.enableReceive(digitalPinToInterrupt(RF_DAT_PIN));
}

uint32_t rfTask(void)
{
  static uint32_t safety_term = 0;

  static uint32_t rx_data = 0;

  if (mySwitch.available()) 
  {
    rx_data = mySwitch.getReceivedValue();
    mySwitch.resetAvailable();
    
    if(remote_mode.mode == REMOTE_LEARN)
    {
      captured_rf_code = rx_data; 
      // 여기서,  모드 조정을 하는게 아닌, 플래그를 세우거나 리턴값으로 유의미한 결과를 가져간다. 
      remote_mode.got_captured_flag = true;
      return rx_data;
    }
    
    uint8_t slot_num = findRemoteSlot(rx_data);
    if(slot_num != RF_UNKNOWN_REMOTE_CONTROLLER)
    {
      // Serial.printf("[RF] 슬롯 %d 리모컨 인식\n", slot_num);
      
      char namespace_name[20];
      sprintf(namespace_name, "remote_%d", slot_num);

      preferences.begin(namespace_name, true);

      remote_mode.up_btn_val = preferences.getUInt("up_btn", 0);
      remote_mode.down_btn_val = preferences.getUInt("down_btn", 0);
      remote_mode.lock_btn_val = preferences.getUInt("lock_btn", 0);
      remote_mode.stop_btn_val = preferences.getUInt("stop_btn", 0);
      
      preferences.end();
      
      if(millis() - safety_term >= REMOTE_BTN_DEBOUNCE_TERM)
      {
        safety_term = millis();

        if(!remote_mode.lock_flag)
        {
          if(rx_data == remote_mode.up_btn_val)
          {
            updateButtonState(BTN_A_PRESSED);
          }
          else if(rx_data == remote_mode.down_btn_val)
          {
            updateButtonState(BTN_B_PRESSED);
          }
          else if(rx_data == remote_mode.stop_btn_val)
          {
            updateButtonState(BTN_D_PRESSED);
          }
       }
      
        if(rx_data == remote_mode.lock_btn_val)
        {
          remote_mode.lock_flag = !remote_mode.lock_flag;
        }
      }
    }
    else
    {
      // Serial.println("[RF] 미등록 리모컨");
      return RF_UNKNOWN_REMOTE_CONTROLLER;
    }
  }

  return RF_NOT_AVAILABLE;
}

void checkCurrentTimeout(float current_val)
{
  static const float adc_threshold = 0.04;
  static bool low_current_detected = false;
  static uint32_t low_current_start_time = 0;

  if (current_val < adc_threshold)
  {
    if (!low_current_detected)
    {
      // 처음 감지
      low_current_start_time = millis();
      low_current_detected = true;
    }
    else
    { 
      if (millis() - low_current_start_time >= LOW_ADC_TIMEOUT)
      {
        motorStop();
        low_current_detected = false;  // 리셋
      }
    }
  }
  else
  {
    // 정상 범위 복귀
    if (low_current_detected)
    {
      low_current_detected = false;
    }
  }
}

void pushLearnSample(void)
{
  if (remote_mode.learn_sample_index == 0)
  {
    remote_mode.first_learn_signal_data = captured_rf_code;
  }

  if (captured_rf_code == remote_mode.first_learn_signal_data && remote_mode.learn_sample_index < LEARN_SAMPLE_MAX) // 끝까지 맨처음 들어온 데이터랑 비교하기.
  {
    remote_mode.learn_samples[remote_mode.learn_sample_index++] = 1;
  }
}

bool isLearnDataStable(void)
{
  Serial.printf("인덱스 : %d\n", remote_mode.learn_sample_index);

  if (remote_mode.learn_sample_index == 0 || remote_mode.learn_sample_index <= MIN_SAMPLE_DATA_NUM)
  {
    return false;
  }
  
  for (uint8_t i = 0; i < remote_mode.learn_sample_index; i++)
  {
    if (remote_mode.learn_samples[i] != 1)
    {
      return false;
    }  
  }
  return true;
}

void resetLearnSamples(void)
{
  memset(remote_mode.learn_samples, 0, sizeof(remote_mode.learn_samples));
  remote_mode.learn_sample_index = 0;
  remote_mode.first_learn_signal_data = 0;
}

void motorGoUp(void)
{
  ledcWrite(MOTOR_IN1_PIN, MOTOR_MIN_SPEED);
  ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
  remote_mode.motor_stoped = false;
}

void motorGoDown(void)
{
  ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
  ledcWrite(MOTOR_IN2_PIN, MOTOR_MIN_SPEED);
  remote_mode.motor_stoped = false;
}

void motorStop(void)
{
  if (!remote_mode.motor_stoped)
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
    remote_mode.motor_stoped = true;
  }
}

void motorSetLock(bool state)
{
  remote_mode.lock_flag = state;
}

bool isMotorLocked(void)
{
  return remote_mode.lock_flag;
}

void allLedOff(void)
{
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
}

void handleWiFiRequests(void)
{
  btn_decoded_t state = BTN_NONE;

  state = wifiTask();

  updateButtonState(state);
}

void updateButtonState(btn_decoded_t state)
{
  static uint32_t chattering_timer = 0;

  if (millis() - chattering_timer < BTN_CHATTERED_TIME)
  {
    chattering_timer = millis();
    return;  
  }

  if(getLastButtonState() == state && state != BTN_D_PRESSED)
  {
    setLastButtonState(BTN_NONE);
    motorStop();
    if (state == BTN_C_PRESSED)
    {
      motorSetLock(UNLOCK);
    }
    return;
  }
  static uint32_t count = 0;
  switch(state)
  {
    case BTN_A_PRESSED:
      if (isMotorLocked() == NO)
      {
        motorGoUp();
        setLastButtonState(BTN_A_PRESSED);
      }
      break;
    
    case BTN_B_PRESSED:
      if (isMotorLocked() == NO)
      {
        motorGoDown();
        setLastButtonState(BTN_B_PRESSED);
      }
      break;

    case BTN_C_PRESSED:
      if (isMotorLocked() == NO)
      {
        motorSetLock(LOCK);
        setLastButtonState(BTN_C_PRESSED);
      }
      break;

    case BTN_D_PRESSED:
      if (isMotorLocked() == NO)
      {
        motorStop();
        setLastButtonState(BTN_D_PRESSED);
      }
      break;
    
    default:
      break;
  }
}

btn_decoded_t getLastButtonState(void)
{
  return remote_mode.last_btn_state;
}

void setLastButtonState(btn_decoded_t state)
{
  remote_mode.last_btn_state = state;
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
    return YES;
  }
  else
  {
    return NO;
  }
}

void userWantsLearnRemote(bool trigger)
{
  remote_mode.btn_flag = trigger;
}

void cliTask(void)
{
  if (Serial.available() > 0) 
  {
    String command = Serial.readStringUntil('\n');
    command.trim();  // 앞뒤 공백/개행 제거
    
    if (command.equalsIgnoreCase("delete")) 
    {
      deleteAllRemotes();
      Serial.println("[명령] 공장 초기화 완료\n");
    }
    else if (command.equalsIgnoreCase("load")) 
    {
      loadAllRemotes();
      Serial.println("[명령] 로드 완료\n");
    }
    else if (command.length() > 0)
    {
      Serial.printf("[명령] 알 수 없는 명령: '%s'\n", command.c_str());
      Serial.println("사용 가능 명령: delete, load");
    }
  }
}
