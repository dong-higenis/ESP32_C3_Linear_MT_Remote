#ifndef REMOTE_WIFI_H
#define REMOTE_WIFI_H

#include <Arduino.h>
#include "main.h"
#include <WiFi.h>

void wifiSetup(void);
btn_decoded_t wifiTask(void);
void httpMakeWeb(WiFiClient &client, const char* ssid);

#endif