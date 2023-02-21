#include "functions.h"

#define PN532_SCK (18)
#define PN532_MISO (19)
#define PN532_MOSI (23)
#define PN532_SS (4)

#define BUZZER (27)
#define GREEN_LED (13)
#define RED_LED (14)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
String idcard;
Ticker getTicker;
Ticker postTicker;

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
  nfc.begin();

  nfcPrintFirmware(nfc);

  nfc.SAMConfig();
  // ##################################################################

  // ##################################################################
  // start http
  // ##################################################################
  // Set up the interrupt for the GET request
  sendGETList();
  getTicker.attach(20, sendGETList); // 60 seconds

  // Set up the interrupt for the POST request
  postTicker.attach(1800, sendPOSTRequest); // 3 minutes
                                            // ##################################################################
}

void loop()
{

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
    Serial.print("ID CARD : ");
    Serial.println(idcard);
  }

  // Convert the hex input to a long integer.
  char *end;
  long idcardLong = strtol(idcard.c_str(), &end, 16);

  Serial.println(idcardLong);
  // Check if the user input matches one of the stored IDs.
  bool found = false;

  for (long id : ids)
  {
    if (id == idcardLong)
    {
      found = true;
      break;
    }
  }
  if (found)
  {
    Serial.println("Can Enter!!!");

    // Set green light toggle and buzzer
    digitalWrite(GREEN_LED, HIGH);
    for (int i = 0; i < 200; i++)
    {
      digitalWrite(BUZZER, HIGH);
      delay(1); // wait for 1ms
      digitalWrite(BUZZER, LOW);
      delay(1); // wait for 1ms
    }

    digitalWrite(GREEN_LED, LOW);
    delay(10); // wait for 1ms
  }
  else
  {
    Serial.println("NO Enter!!!");
    // set red light toggle
    digitalWrite(RED_LED, HIGH);
    delay(500); // wait for 1ms
    digitalWrite(RED_LED, LOW);
    delay(10); // wait for 1ms
  }

  delay(100);
}
