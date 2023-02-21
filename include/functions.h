#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>

#include <vector>

//  Functions

extern portMUX_TYPE timerMux;
extern String url_client;

extern const char *ssid;
extern const char *password;
extern String port;
extern uint64_t chipid;

extern std::vector<long> ids;

// ##################################################################
// print chips id
// ##################################################################
void printChipId();

// ##################################################################
// function that prints firmware version of PN532 card
// ##################################################################
void nfcPrintFirmware(Adafruit_PN532 nfc);

// ##################################################################
// connect to wifi
// ##################################################################
void WifiConnect(const char *ssid, const char *password);

// ##################################################################
// on wifi disconnect
// ##################################################################
void IRAM_ATTR onWiFiEvent(WiFiEvent_t event);

// ##################################################################
// Get ID list request
// ##################################################################
void sendGETList();

// ##################################################################
// POST request
// ##################################################################
void sendPOSTRequest();

#endif