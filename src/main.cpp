#include "functions.h"

// Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
Desfire nfc_PN532;
String idcard;
Ticker getTicker;
Ticker postTicker;

AES nfc_PiccMasterKey; // An authentication key for the given cards. Defined in Secrets.h
AES encryption;
bool initSuccess = false;
uint64_t lastId = 0;


SPIClass hspi(HSPI);

void setup()
{
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  // ##################################################################
  // Print ESP32 ID
  // ##################################################################
  printChipId();

  // ##################################################################
  // connect to WIFI
  // ##################################################################
  WiFi.onEvent(onWiFiEvent);
  WifiConnect(ssid, password);
  // Wait for the connection to be established
  delay(1000);

  // ##################################################################
  // connect to PN532 via SPI
  // ##################################################################
  nfc_PN532.InitHardwareSPI(SPI_CS_PIN, RESET_PIN);

  // ##################################################################
  // initialize the SD card
  // ##################################################################

  hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, hspi))
  {
    Serial.println("Error initializing SD card.");
  }

  // Check if admittance_list.txt exists, and create it if it doesn't
  check_make_file(Permitted_ID_LIST_file);
  check_make_file(LOG_file);

  // ##################################################################
  // start http
  // ##################################################################
  // load file to set
  LoadFileToIDSet();

  // ##################################################################
  // start http
  // ##################################################################
  // get request
  sendGETList();
  // Set up the interrupt for the GET request
  getTicker.attach(120, sendGETList); // 60 seconds

  // Set up the interrupt for the POST request
  postTicker.attach(1800, sendPOSTRequest);
  // ##################################################################

  // Setting up the master key of the PICC
  nfc_PiccMasterKey.SetKeyData(SECRET_PICC_MASTER_KEY, sizeof(SECRET_PICC_MASTER_KEY), CARD_KEY_VERSION);

  // get the real time
  SendGetTime();

  delay(1000);
  time_t now = time(NULL);
  Serial.println(ctime(&now));

  // Start threads for listening to server and processing input
  xTaskCreatePinnedToCore(listenAndSave, "ListenAndSave", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(processInput, "ProcessInput", 10000, NULL, 1, NULL, 0);
}

void loop()
{

 
}
