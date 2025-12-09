
#include "main.h"

/**
 * @brief 리모컨 데이터 샘플링 상수
 */
#define LEARN_SAMPLE_MAX 50
#define MIN_SAMPLE_DATA_NUM 10

/**
 * @brief LED BLINK 시간 정의
 */
#define FAST_LED 100
#define REMOTE_LEARN_MODE_INTERVEL 500
#define REMOTE_LEARN_RECOGNIZED_INTERVEL 100

/**
 * @brief 타이머 상수
 */
#define PRESSED_TIME_FOR_DELETE_MODE 10000
#define PRESSED_TIME_FOR_LEARN_MODE 3000
#define LEARN_MODE_TIMEOUT 10000
#define LEARN_SUCCEED_CATURE_TERM 5000
#define LEARN_COMPLETE_TERM 3000
#define GO_IDLE_AFTER_3000MS 3000
#define DO_DELETE_AFTER_2000MS 2000

typedef struct 
{
  remote_state_t mode;
  uint32_t captured_data;
  btn_decoded_t last_btn_state;

  uint32_t first_learn_signal_data;
  uint8_t  learn_samples[LEARN_SAMPLE_MAX];
  uint8_t  learn_sample_index;

  uint32_t timeout_timer;

} remote_mode_t;

remote_mode_t remote_mode = {REMOTE_IDLE, 0, BTN_NONE, 0, {0}, 0, 0};

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
  handlePhysicButtonPressLogic();
  adcTask();
  modeUpdate();
  rfTask();
  cliTask();
  handleWiFiRequests();
}

void modeUpdate(void)
{
  static uint8_t result = 0;
  static uint32_t succeed_timer = RESET_TIMER;
  static uint32_t done_timer = RESET_TIMER;
  
  switch(remote_mode.mode)
  {
    case REMOTE_IDLE:
      allLedOff();
      break;
    
    case REMOTE_LEARN:
      ledTask(REMOTE_LEARN_MODE_INTERVEL);

      if (isCapturedRfSignal())
      {
        remote_mode.captured_data = getCapturedData();
        pushLearnSample(remote_mode.captured_data); // 데이터가 들어올 때 마다, 배열에 데이터 삽입
        goCaptureNextSignal();
        
        if (millis () - remote_mode.timeout_timer >= LEARN_SUCCEED_CATURE_TERM) // 시계 여기서 체크
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
      if (millis() - remote_mode.timeout_timer >= LEARN_MODE_TIMEOUT)
      {
        remote_mode.mode = REMOTE_IDLE;  
      }
      break;
    
    case REMOTE_CAPTURED: // 여기서 완료되면 빠르게 깜빡
      ledTask(REMOTE_LEARN_RECOGNIZED_INTERVEL);
      
      motorSetLock(); // 이미 등록된 모터가 움직이지 않도록 lock을 건다.

      if(millis() - succeed_timer >= LEARN_COMPLETE_TERM)
      {
        result = learnNewRemote(remote_mode.captured_data);
        
        if(result == SUCCEED)
       {
         Serial.println("[MODE] 저장 완료!");
         motorSetUnlock();

         remote_mode.mode = REMOTE_DONE;
       }
       else if(result == RF_ALREADY_EXIST)
       {
         Serial.println("[MODE] 이미 등록됨");
         motorSetUnlock();

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
        motorSetUnlock();
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
        motorSetLock();
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

void handlePhysicButtonPressLogic(void)
{
  static bool btn_a_was_pressed = false;
  static bool btn_b_was_pressed = false;

  uint32_t press_duration = getButtonPressDuration();
  btn_decoded_t state = getButtonState();

  switch (state)
  {
    case BTN_PHYSIC_BOTH_PRESSED:
    
    // 10초 이상: 공장 초기화
    if (press_duration >= PRESSED_TIME_FOR_DELETE_MODE) 
    {
      if (remote_mode.mode == REMOTE_LEARN)
      {
        remote_mode.mode = REMOTE_DELETE;
        unlockButtonFlag();
      }
    }

    // 3초 이상: 학습 모드
    if ( (press_duration >= PRESSED_TIME_FOR_LEARN_MODE) && (press_duration < PRESSED_TIME_FOR_DELETE_MODE) )
    {
      Serial.println("학습모드 진입");
      if (remote_mode.mode == REMOTE_IDLE) 
      {
        remote_mode.mode = REMOTE_LEARN;
        remote_mode.timeout_timer = millis();
        Serial.println("리모컨 등록을 원하실 경우, 리모컨 버튼을 꾹 누르고 계십시오.");
      }
    }
      break;

    case BTN_A_PRESSED:
      btn_a_was_pressed = true;
      break;
    case BTN_B_PRESSED:
      btn_b_was_pressed = true;
      break;
    case BTN_NONE:
      unlockButtonFlag();
      if (btn_a_was_pressed)
      {
        updateButtonState(BTN_A_PRESSED);
        btn_a_was_pressed = false;
      }
      else if (btn_b_was_pressed)
      {
        updateButtonState(BTN_B_PRESSED);
        btn_b_was_pressed = false;
      }
      break;
    default:
      break;
  }
}

void pushLearnSample(uint32_t captured_rf_code)
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

void handleWiFiRequests(void)
{
  btn_decoded_t state = BTN_NONE;

  state = wifiTask();

  updateButtonState(state); 
}

void updateButtonState(btn_decoded_t state)
{
  if(getLastButtonState() == state && state != BTN_D_PRESSED)
  {
    setLastButtonState(BTN_NONE);
    motorStop();
    if (state == BTN_C_PRESSED)
    {
      motorSetUnlock();
    }
    return;
  }

  switch(state)
  {
    case BTN_A_PRESSED:
      if (isMotorLocked() == false)
      {
        motorGoUp();
        setLastButtonState(BTN_A_PRESSED);
      }
      break;
    
    case BTN_B_PRESSED:
      if (isMotorLocked() == false)
      {
        motorGoDown();
        setLastButtonState(BTN_B_PRESSED);
      }
      break;

    case BTN_C_PRESSED:
      if (isMotorLocked() == false)
      {
        motorSetLock();
        setLastButtonState(BTN_C_PRESSED);
      }
      break;

    case BTN_D_PRESSED:
      if (isMotorLocked() == false)
      {
        motorStop();
        setLastButtonState(BTN_D_PRESSED);
      }
      break;
    
    default:
      break;
  }
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

btn_decoded_t getLastButtonState(void)
{
  return remote_mode.last_btn_state;
}

void setLastButtonState(btn_decoded_t state)
{
  remote_mode.last_btn_state = state;
}

remote_state_t getRemoteModeState(void)
{
  return remote_mode.mode;
}

void setRemoteModeState(remote_state_t state)
{
  remote_mode.mode = state;
}
