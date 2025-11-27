/**
*@brief Pin 번호
*/
#define BTN1_PIN 0
#define BTN2_PIN 4

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  btnSetup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  btnTask();
}

/**
*@brief 버튼핀을 입력 풀업으로 설정 한다.
*/
void btnSetup(void)
{
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
}

/**
*@brief 실시간으로 버튼상태를 변수에 넣으나, Serial Monitor에 출력할 버튼 상태는 500ms마다 진행한다.
*/
void btnTask(void)
{
  static bool btn1_state = HIGH;
  static bool btn2_state = HIGH;
  static uint32_t btn_timer = 0;

  btn1_state = digitalRead(BTN1_PIN);
  btn2_state = digitalRead(BTN2_PIN);

  if (millis() - btn_timer >= 500)
  {
    Serial.printf(" BTN1 [%d] BTN2 [%d]\n", btn1_state, btn2_state);
  }
}