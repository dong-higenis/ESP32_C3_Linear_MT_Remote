
/**
* @note ---**사용 라이브러리**---
*       NAME:rc-switch by sui77 
*       VERSION: 2.6.4
*/

#include <RCSwitch.h>

/**
* @brief Pin 번호
*/
#define MOTOR_IN1_PIN 18
#define MOTOR_IN2_PIN 19
#define RF_DAT_PIN 10

#define MOTOR_MIN_SPEED 255
#define MOTOR_STOP_SPEED 0 

/**
* @brief 리모컨 버튼 value
* @attention 실제로 돌려보고 자신이 가진 리모컨의 버튼값을 Serial monitor에서 확인 후 기입!!
*/
#define REMOTE_CON_A_BUTTON 0xFFF1 // 향후 수정
#define REMOTE_CON_B_BUTTON 0xFFF2 // 향후 수정
#define REMOTE_CON_C_BUTTON 0xFFF3 // 향후 수정
#define REMOTE_CON_D_BUTTON 0xFFF4 // 향후 수정

/**
 * @brief RCSwitch 라이브러리로 RF 리모컨 신호를 송수신하는 객체 생성
 *        - 433/315MHz 무선 신호(리모컨/무선 소켓 등)를 Arduino/ESP32에서 처리할 수 있게 해준다.
 *        - mySwitch 객체를 통해 송신(send)·수신(receive) 기능 사용
 */
RCSwitch mySwitch = RCSwitch();

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  motorSetup();
  rfSetup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  rfTask();
}
/**
 * @brief RF 리모컨 수신기 초기화 함수
 *        RF 데이터 핀을 인터럽트 방식으로 설정하여 리모컨 신호 수신을 시작한다.
 */
void rfSetup(void)
{
  mySwitch.enableReceive(digitalPinToInterrupt(RF_DAT_PIN));
}

/**
 * @brief RF 리모컨 신호 수신 및 명령 처리 함수
 *        - 리모컨에서 신호가 오면, 코드를 읽어서  
 *          각 코드(버튼)에 따라 모터를 위/아래로 움직이거나 멈춘다.
 *        - 리모컨 코드는 시리얼로 출력된다.
 */
void rfTask(void)
{
  if (mySwitch.available()) 
  {
    uint32_t receivedCode = mySwitch.getReceivedValue();
    Serial.print("RF Code: "); 
    Serial.println(receivedCode);

    if (receivedCode == REMOTE_CON_A_BUTTON) 
    {
      ledcWrite(MOTOR_IN1_PIN, MOTOR_MIN_SPEED);
      ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
    }
    else if (receivedCode == REMOTE_CON_B_BUTTON)
    {
      ledcWrite(MOTOR_IN2_PIN, MOTOR_MIN_SPEED);
      ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    }
    else if (receivedCode == REMOTE_CON_C_BUTTON)
    {
      ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
      ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    }

    mySwitch.resetAvailable();
  }
}

void motorSetup(void)
{
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);

  ledcAttach(MOTOR_IN1_PIN, 5000, 8);
  ledcAttach(MOTOR_IN2_PIN, 5000, 8);
}