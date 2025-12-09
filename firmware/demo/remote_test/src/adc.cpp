#include "Inc/adc.h"
#include "Inc/motor.h"

void adcSetup(void)
{
  analogReadResolution(12);
}

void adcTask(void)
{
  static uint32_t raw_adc = 0;
  static float motor_voltage = 0;
  static float motor_current = 0;
  
  raw_adc = analogRead(MOTOR_CURRENT_ADC_PIN);
  motor_voltage = (raw_adc / ADC_MAX_VALUE) * INPUT_VOLTAGE;
  float raw_current = motor_voltage / MOTOR_GAIN;
  
  static float filtered_current = 0;
  filtered_current = filtered_current * 0.9 + raw_current * 0.1; // 9:1로 LOW PASS FILTER, 필터링 처리된 값이 9할
  
  motor_current = filtered_current;
  
  checkCurrentTimeout(motor_current);
}

void checkCurrentTimeout(float current_val)
{
  static bool low_current_detected = false;
  static uint32_t low_current_start_time = 0;

  if (current_val < MOTOR_ADC_THRESHOLD) // 센서가 측정한 전류값이 모터가 움직이는 상태의 전류범위 미만에 있을경우
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
  else if (current_val >= MOTOR_ADC_THRESHOLD) // 움직이는 상태
  {
    if (low_current_detected)
    {
      low_current_detected = false;
    } 
  }
}