
/**
*@brief Pin번호
*/
#define LED_PIN 2
#define LED2_PIN 1

void setup() 
{
  // put your setup code here, to run once:
  ledSetup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  ledTask();
}

/**
*@brief LEN 1번, 2번핀을 OFF상태로 초기화
*/
void ledSetup(void)
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
}

/**
*@brief LEN 1번, 2번핀을 500ms마다 번갈아가며 토글한다.
*/
void ledTask(void)
{
  static uint32_t led_timer = 0;
  static bool led_state = HIGH;
  
  if(millis() - led_timer >= 500)
  {
    led_timer = millis();
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    digitalWrite(LED2_PIN, !led_state);
  }
}
