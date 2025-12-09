#include "Inc/led.h"

bool is_led_off = false;

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

void allLedOff(void)
{
  if (!is_led_off)
  {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    is_led_off = true;
  }
}