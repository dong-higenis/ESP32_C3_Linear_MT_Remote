
/**
*@brief Pin 번호
*/
#define BTN1_PIN 0
#define BTN2_PIN 4
#define MOTOR_IN1_PIN 18
#define MOTOR_IN2_PIN 19
#define MOTOR_CURRENT_ADC_PIN 3

/**
*@brief 모터 PWM 속도 정의 ( 8비트 해상도 최대값 = 255 )
*/
#define MOTOR_MIN_SPEED 255
#define MOTOR_STOP_SPEED 0 

/**
*@brief 모터 Gain값 (변환비)
*/
#define MOTOR_GAIN 0.554f

/**
*@brief 버튼 입력 상태 변수
*/
bool btn1_state = HIGH;
bool btn2_state = HIGH;

/**
*@brief 모터 소비 전류값
*/
float motor_current = 0.0f;

void setup() 
{
  // put your setup code here, to run once:  
  btnSetup();
  motorSetup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  btnTask();
  motorTask();
}

/**
*@brief  ESP32에서 ADC(아날로그 to 디지털)의 해상도를 설정하는 초기화 함수 ( 12bit: 0 - 4095 )
*/
void adcSetup(void)
{
  analogReadResolution(12);
}

/**
*@brief    3.3V 기준 전압으로 변환한 뒤, 모터 전류 측정에 필요한 전류값으로 계산하는 함수(12비트 ADC 해상도 기준).
*/
void adcTask(void)
{
  static uint32_t raw_adc = 0;
  static float motor_voltage = 0;

  raw_adc = analogRead(MOTOR_CURRENT_ADC_PIN); // - analogRead()로 12비트(0~4095) ADC 원시값 읽기
  motor_voltage = (raw_adc / 4095.0) * 3.3; // - 원시값을 전압(0~3.3V)으로 스케일 변환
  motor_current = motor_voltage / MOTOR_GAIN; // - 전압을 모터 전류 센서의 증폭기 이득으로 나누어 실제 전류 계산
}

void motorSetup(void)
{
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);

  ledcAttach(MOTOR_IN1_PIN, 5000, 8);
  ledcAttach(MOTOR_IN2_PIN, 5000, 8);
}

void motorTask(void)
{
  if (!btn1_state && btn2_state) 
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_MIN_SPEED); 
    ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED); 
  } 
  else if (!btn2_state && btn1_state) 
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_MIN_SPEED); 
  } 
  else if (btn1_state && btn2_state)
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
  }
}

void btnSetup(void)
{
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
}

void btnTask(void)
{
  btn1_state = digitalRead(BTN1_PIN);
  btn2_state = digitalRead(BTN2_PIN);
}
