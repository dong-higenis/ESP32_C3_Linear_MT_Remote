#ifndef RF_H
#define RF_H

#include "main.h"

#define REMOTE_BTN_DEBOUNCE_TERM 300

void rfSetup(void);
uint32_t rfTask(void);
bool isCapturedRfSignal(void);
void goCaptureNextSignal(void);
uint32_t getCapturedData(void);

#endif