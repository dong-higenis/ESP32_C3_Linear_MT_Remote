
/**
*@brief Pin 번호
*/
#define BTN1_PIN 0
#define BTN2_PIN 4
#define MOTOR_IN1_PIN 18
#define MOTOR_IN2_PIN 19

/**
*@brief 모터 PWM 속도 정의 ( 8비트 해상도 최대값 = 255 )
*/
#define MOTOR_MIN_SPEED 255
#define MOTOR_STOP_SPEED 0 

/**
*@brief 버튼 입력 상태 변수
*/
bool btn1_state = HIGH;
bool btn2_state = HIGH;

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
*@brief ESP전용 PWM 컨트롤러 이름 = LEDC, 즉 두 핀에 PWM을 활성화 시켜준다. 각 인자는 (핀번호, 주파수, 해상도) 이다. (여기선 8비트 해상도)
*/
void motorSetup(void)
{
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);

  ledcAttach(MOTOR_IN1_PIN, 5000, 8);
  ledcAttach(MOTOR_IN2_PIN, 5000, 8);
}

/**
*@brief 1번 버튼을 누르면 모터는 위로, 2번 버튼을 누르면 모터는 아래로 내려간다. 버튼을 누르지 않으면 모터는 멈춘다.
*       
*/
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
