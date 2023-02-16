#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>

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

#endif