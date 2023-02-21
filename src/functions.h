#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "time.h"

//  Functions

extern portMUX_TYPE timerMux;
extern String url_client;

extern const char *ssid;
extern const char *password;
extern String port;
extern uint64_t chipid;

void WifiConnect(const char *ssid, const char *password);

void IRAM_ATTR onWiFiEvent(WiFiEvent_t event);

void sendGETList();

void sendPOSTRequest();

bool isIdAllowed(String id);

bool isIdAllowedOnLI(String id);

void printLocalTime();

void SendGetTime();

#endif