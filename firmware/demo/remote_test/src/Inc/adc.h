#ifndef ADC_H
#define ADC_H

#include "main.h"

/**
* @brief 모터 상수
*/
#define INPUT_VOLTAGE 3.3f

/**
* @brief 모터 상수
*/
#define MOTOR_GAIN 0.554f
#define MOTOR_ADC_THRESHOLD 0.04f

/**
* @brief adc 관련 상수
*/
#define ADC_12BITS_RESOLUTION 12
#define ADC_MAX_VALUE 4095.0f

#define LOW_ADC_TIMEOUT 3000

void adcSetup(void);
void adcTask(void);
void checkCurrentTimeout(float current_val);

#endif