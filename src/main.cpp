#include "functions.h"

#define PN532_SCK (18)
#define PN532_MISO (19)
#define PN532_MOSI (23)
#define PN532_SS (4)

int red_LED = 12;
int grean_LED =13;
#define PN532_SS (4)

#define FILENAME "test.txt"
int forwrite = 0;

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

  // // ##################################################################
  // // start http
  // // ##################################################################
  // // Set up the interrupt for the GET request
  // getTicker.attach(60, sendGETList); // 60 seconds

  // // Set up the interrupt for the POST request
  // postTicker.attach(1800, sendPOSTRequest); // 3 minutes

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount file system");
    return;
  }

  // Create a new file for ID list input
  File file_data = SPIFFS.open("/list_data.txt", FILE_WRITE);
  if (!file_data)
  {
    Serial.println("Failed to create file");
    return;
  }
  // Create a new file for LOG input 
  File file_log = SPIFFS.open("/log.txt", FILE_WRITE);
  if (!file_log)
  {
    Serial.println("Failed to create file");
    return;
  }

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
  pinMode (grean_LED, OUTPUT);
   pinMode (red_LED, OUTPUT);

}

void loop()
{
  // put your main code here, to run repeatedly:
  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
  uint8_t uidLength;

  // if(forwrite==0){

  //   // Define the data to write to the tag
  //   uint8_t data1[] = { 0x09, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
  //   // Write the data to the tag
  //   uint8_t success1 = nfc.mifareclassic_WriteDataBlock(0, data1);

  //   if (success1) {
  //     forwrite = 1;
  //     Serial.println("Write successful!");
  //   } else {
  //     Serial.println("Write failed.");
  //   }
  // }
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success)
  {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ...
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

      // Now we need to try to authenticate it for read/write access
      // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      Serial.println("Trying to authenticate block 4 with default KEYA value");
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	  // Start with block 4 (the first block of sector 1) since sector 0
	  // contains the manufacturer data and it's probably better just
	  // to leave it alone unless you know what you're doing
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);

      if (success)
      {
        Serial.println("Sector 1 (Blocks 4..7) has been authenticated");
        uint8_t data[16];

        // If you want to write something to block 4 to test with, uncomment
		    // the following line and this text should be read back in a minute

        // memcpy(data, (const uint8_t[]){ 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0 }, sizeof data);
        // success = nfc.mifareclassic_WriteDataBlock (4, data);

        // Try to read the contents of block 4
        success = nfc.mifareclassic_ReadDataBlock(4, data);

        if (success)
        {
          // Data seems to have been read ... spit it out
          Serial.println("Reading Block 4:");
          nfc.PrintHexChar(data, 16);
          Serial.println("");

          // Wait a bit before reading the card again
          delay(1000);
        }
        else
        {
          Serial.println("Ooops ... unable to read the requested block.  Try another key?");
        }
      }
      else
      {
        Serial.println("Ooops ... authentication failed: Try another key?");
      }
    }

    if (uidLength == 7)
    {
      // We probably have a Mifare Ultralight card ...
      Serial.println("Seems to be a Mifare Ultralight tag (7 byte UID)");

      // Try to read the first general-purpose user page (#4)
      Serial.println("Reading page 4");
      uint8_t data[32];
      success = nfc.mifareultralight_ReadPage (4, data);
      if (success)
      {
        // Data seems to have been read ... spit it out
        nfc.PrintHexChar(data, 4);
        Serial.println("");

        // Wait a bit before reading the card again
        delay(1000);
      }
      else
      {
        Serial.println("Ooops ... unable to read the requested page!?");
      }
    }



    // // Display some basic information about the card
    // idcard = "";
    // for (byte i = 0; i <= uidLength - 1; i++)
    // {
    //   idcard += (uid[i] < 0x10 ? "0" : "") +
    //             String(uid[i], HEX);
    // }
    // Serial.print("ID CARD in HEX : ");
    // Serial.println(idcard);
    //    // read data from block 4
    // uint8_t data[16];
    // success = nfc.mifareclassic_ReadDataBlock(4, data);

    // if (success) {
    //   Serial.println("Read data:");
    //   for (int i = 0; i < 16; i++) {
    //     Serial.print(data[i], HEX);
    //     Serial.print(" ");
    //   }
    //   Serial.println("");
    // } else {
    //   Serial.println("Read data failed");
    // }
  

    // // compare with saved data

    // if (isIdAllowed(idcard))
    // {
    
    //   Serial.println("Access granted");
    //   // add code here to grant access
    //   digitalWrite(grean_LED, HIGH);
    //   delay(3000);
    //   digitalWrite(grean_LED, LOW);
    // }
    // else
    // {
    
    //   Serial.println("Access denai");
    //   digitalWrite(red_LED, HIGH);
    //   delay(3000);
    //   digitalWrite(red_LED, LOW);
    // }
  }
}