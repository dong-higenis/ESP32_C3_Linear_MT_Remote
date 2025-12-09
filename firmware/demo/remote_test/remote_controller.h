#ifndef REMOTE_CONTROLLER_H
#define REMOTE_CONTROLLER_H

#include <Arduino.h>
#include "main.h"
#include <Preferences.h>
#define ATYPE_REMOTE_UP_BTN 0x04
#define ATYPE_REMOTE_DOWN_BTN 0x08
#define ATYPE_REMOTE_LOCK_BTN 0x01
#define ATYPE_REMOTE_STOP_BTN 0x02

#define BTYPE_REMOTE_A_BTN 0x01
#define BTYPE_REMOTE_B_BTN 0x02
#define BTYPE_REMOTE_C_BTN 0x04
#define BTYPE_REMOTE_D_BTN 0x08

void remoteSetup(void);
void loadAllRemotes(void);
uint8_t learnNewRemote(uint32_t code);
int findRemoteSlot(uint32_t receivedCode);
void deleteLastRemote(void);
void deleteAllRemotes(void);

#endif