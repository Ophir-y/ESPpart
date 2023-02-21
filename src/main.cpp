#include "functions.h"

#define PN532_SCK (18)
#define PN532_MISO (19)
#define PN532_MOSI (23)
#define PN532_SS (4)

#define FILENAME "test.txt"


Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
String idcard;
Ticker getTicker;
Ticker postTicker;

void setup()
{
  Serial.begin(115200);
  String chipids = String(chipid);
  Serial.printf("ESP32 Chip ID In DEC: ");
  Serial.println(chipids);                                                                       // print the chip ID
  Serial.printf("ESP32 Chip ID In HEX: %04X%08X\n", (uint16_t)(chipid >> 32), (uint32_t)chipid); // print the chip ID
  WiFi.onEvent(onWiFiEvent);
  WifiConnect(ssid, password);
  // Wait for the connection to be established
  delay(1000);
  // ##################################################################
  // connect to PN532 via SPI
  // ##################################################################
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    Serial.print("Didn't find PN53x board");
    while (1)
      ; // halt
  }
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);
  nfc.SAMConfig();
  // ##################################################################

  // ##################################################################
  // start http
  // ##################################################################
  // Set up the interrupt for the GET request
  getTicker.attach(50, sendGETList); // 60 seconds

  // Set up the interrupt for the POST request
  postTicker.attach(1800, sendPOSTRequest); // 3 minutes

  // // Initialize SPIFFS
  // if (!SPIFFS.begin()) {
  //   Serial.println("Failed to mount file system");
  //   return;
  // }

  // // Create a new file
  // File file = SPIFFS.open("/list_data.txt", FILE_WRITE);
  // if(!file){
  //   Serial.println("Failed to create file");
  //   return;
  // }

  //   // initialize SD card
  // if (!SD.begin()) {
  //   Serial.println("Error initializing SD card");
  //   return;
  // }

  //   // open file for writing
  // File file = SD.open(FILENAME, FILE_WRITE);
  // if (!file) {
  //   Serial.println("Error opening file");
  //   return;
  // }

  // get the real time
  SendGetTime();

}

void loop()
{
  // put your main code here, to run repeatedly:
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success)
  {
    // Display some basic information about the card
    idcard = "";
    for (byte i = 0; i <= uidLength - 1; i++)
    {
      idcard += (uid[i] < 0x10 ? "0" : "") +
                String(uid[i], HEX);
    }
    Serial.print("ID CARD in HEX : ");
    Serial.println(idcard);
    // compare with saved data
    // if (isIdAllowed(idcard)) {
    //   Serial.println("Access granted");
    //   // add code here to grant access
    // }
    // else{
    //   Serial.println("Access denai");
    // }
  }
   time_t now = time(NULL);
  Serial.println(ctime(&now));
  
  delay(1000);
}