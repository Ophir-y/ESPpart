#include "functions.h"

// ########   SPI for PN532:
#define PN532_SCK (18)
#define PN532_MISO (19)
#define PN532_MOSI (23)
#define PN532_SS (4) //

// ########   SPI for SD reader:
#define SD_SCK (14)
#define SD_MISO (26)
#define SD_MOSI (13)
#define SD_CS (27)

#define BUZZER (25)
#define GREEN_LED (33)
#define RED_LED (32)

// #define WATCHDOG_TIMEOUT_S 3

// Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
Desfire nfc_PN532;
String idcard;
Ticker getTicker;
Ticker postTicker;

AES nfc_PiccMasterKey; // An authentication key for the given cards. Defined in Secrets.h
bool initSuccess = false;
uint64_t lastId = 0;
int keep_alive_counter = 0;

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
  // nfc_PN532.InitHardwareSPI(SPI_CS_PIN, RESET_PIN);
  // (byte u8_Clk, byte u8_Miso, byte u8_Mosi, byte u8_Sel, byte u8_Reset)
  nfc_PN532.InitSoftwareSPI(PN532_SCK, PN532_MISO, PN532_MOSI, SPI_CS_PIN, RESET_PIN);
  nfc_PN532.begin();

  delay(50);

  // ##################################################################
  // initialize the SD card
  // ##################################################################

  hspi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, hspi))
  {
    Serial.println("Error initializing SD card.");
  }
  delay(50);
  // // ##################################################################
  // // watchDogTimer
  // // ##################################################################

  // watchDogTimer = timerBegin(2, 80, true);
  // timerAttachInterrupt(watchDogTimer, &watchDogInterrupt, true);
  // timerAlarmWrite(watchDogTimer, WATCHDOG_TIMEOUT_S * 1000000, false);
  // timerAlarmEnable(watchDogTimer);
  // esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);

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
}

void loop()
{

  long id = TrigerRfid();

  if (id < 0)
  {
    return; // Failure
  }
  else if (id > 0)
  { // Card was presented
    String message = String(id);
    // long idl = (long)id;
    String isApprov;
    if (isApproved(id))
    {
      isApprov = "Approved";
      message += " " + isApprov;
      Serial.println(message);
      digitalWrite(GREEN_LED, HIGH);
      for (int i = 0; i < 200; i++)
      {
        digitalWrite(BUZZER, HIGH);
        delay(1); // wait for 1ms
        digitalWrite(BUZZER, LOW);
        delay(1); // wait for 1ms
      }
      digitalWrite(GREEN_LED, LOW);
      delay(100);
      return;
    }
    else
    {
      isApprov = "Declined";
      message += " " + isApprov;

      Serial.println(message);
      digitalWrite(RED_LED, HIGH);
      for (int i = 0; i < 200; i++)
      {
        digitalWrite(BUZZER, HIGH);
        delay(2); // wait for 1ms
        digitalWrite(BUZZER, LOW);
        delay(1); // wait for 1ms
      }
      digitalWrite(RED_LED, LOW);
      delay(100);
      return;
    }
  }
  delay(100);
}
