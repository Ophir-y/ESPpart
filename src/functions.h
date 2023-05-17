#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
// #include <esp_task_wdt.h>
#include <sdkconfig.h>
#include <iostream>
// #include <Adafruit_PN532.h>
#include <HTTPClient.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "adafruit.h"
#include <set>
#include <SD.h>
#include "time.h"

#include "mbedtls/aes.h"
#include "Cipher.h"

#include "dooranser.h"

// Define the maximum number of IDs the door can store
#define MAX_IDS 1000

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

// extern std::set<long> check_ids;
// extern std::set<long> ids;

// Define the structure to hold ID data
struct idData
{
    int id;
    int startTime;
    int endTime;
};

// Define the array to hold ID data
extern idData ids[MAX_IDS];
extern idData check_ids[MAX_IDS];

// ##################################################################
// print chips id
// ##################################################################
void printChipId();

// // ##################################################################
// // function that prints firmware version of PN532 card
// // ##################################################################
// void nfcPrintFirmware(Adafruit_PN532 nfc);

// void watchDogRefresh();

// void IRAM_ATTR watchDogInterrupt();

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
// Check if an ID is approved based on the current time
bool isApproved(int id);

int convertTimeToInt(const char *timeStr);

void saveinlog(String isApproved, int id);

String ReadFile(String whichfile);

#endif