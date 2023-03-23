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
#include <SD.h>
#include "time.h"
#include <stdio.h>


#include <Cipher.h>
#include "mbedtls/aes.h"
#include "Cipher.h"


#include "partosta.h"

// #include <AESLib.h>
// #include "AES.h"



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

extern std::set<long> check_ids;
extern std::set<long> ids;

// int keep_alive_counter = 0;

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

void listenAndSave(void * parameter);

void processInput(void * parameter);

  // Encrypt the data on the SD card
void encryptSDCard();

  // Decrypt the data on the SD card
void decryptSDCard();

#endif