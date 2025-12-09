#ifndef LED_H
#define LED_H

#include "main.h"

void ledSetup(void);
void ledTask(uint32_t led_task_intervel);
void allLedOff(void);

#endif