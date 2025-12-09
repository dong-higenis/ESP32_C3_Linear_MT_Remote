#ifndef BUTTON_H
#define BUTTON_H

#include "main.h"

void btnSetup(void);
uint32_t getButtonPressDuration(void);
btn_decoded_t getButtonState(void);
bool IsAllBtnsPressing(void);
void unlockButtonFlag(void);

#endif