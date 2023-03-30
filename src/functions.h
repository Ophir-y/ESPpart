#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
// #include <Adafruit_PN532.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "adafruit.h"
#include <set>
#include <vector>
#include <SD.h>
#include "time.h"
#include "Classes.h"

//  Functions

extern portMUX_TYPE timerMux;
extern String url_client;

// WIFI info
extern const char *ssid;
extern const char *password;
extern String port;

// chip id
extern uint64_t chipid;

// file path on ESP
extern String Permitted_ID_LIST_file;
extern String LOG_file;

extern std::unique_ptr<AllowedID> check_ids;
extern std::unique_ptr<AllowedID> ids;

// ##################################################################
// print chips id
// ##################################################################
void printChipId();

// // ##################################################################
// // function that prints firmware version of PN532 card
// // ##################################################################
// void nfcPrintFirmware(Adafruit_PN532 nfc);

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

void printLocalTime();

void SendGetTime();

// **********************************************************************************
// initialize the SD card
// ##################################################################
// check if file exists, if not create one
// input: file name
// ##################################################################
void check_make_file(String file_name);
// **********************************************************************************
void LoadFileToIDSet();
#endif