#include "functions.h"
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
std::set<int> ids;
// const char *ssid = "project_wifi";
// const char *password = "12345678";
const char *ssid = "OphirBZK";
const char *password = "noop2802";
// const char* ssid = "Rothsl";
// const char* password = "Bana&nitzan";

uint64_t chipid = ESP.getEfuseMac();
String port = "1231";

String url_client = "http://192.168.1.69:" + port;

// ##################################################################
// print chips id
// ##################################################################

void printChipId()
{
    String chipids = String(chipid);
    Serial.printf("ESP32 Chip ID In DEC: ");
    Serial.println(chipids);                                                                       // print the chip ID
    Serial.printf("ESP32 Chip ID In HEX: %04X%08X\n", (uint16_t)(chipid >> 32), (uint32_t)chipid); // print the chip ID
}

// // ##################################################################
// // function that prints firmware version of PN532 card
// // ##################################################################
// void nfcPrintFirmware(Adafruit_PN532 nfc1)
// {
//     uint32_t versiondata = nfc1.getFirmwareVersion();
//     if (!versiondata)
//     {
//         Serial.print("Didn't find PN53x board");
//         while (1)
//             ; // halt
//     }
//     Serial.print("Found chip PN5");
//     Serial.println((versiondata >> 24) & 0xFF, HEX);
//     Serial.print("Firmware ver. ");
//     Serial.print((versiondata >> 16) & 0xFF, DEC);
//     Serial.print('.');
//     Serial.println((versiondata >> 8) & 0xFF, DEC);
// }

// ##################################################################
// connect to wifi
// ##################################################################
void WifiConnect(const char *ssid, const char *password)
{
    WiFi.mode(WIFI_STA); // Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");
    for (int i = 0; i < 40; i++)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(100);
        }
        else
        {
            Serial.println("\nConnected to the WiFi network");
            Serial.print("Local ESP32 IP: ");
            Serial.println(WiFi.localIP());
            return;
        }
    }
    Serial.println("");
    Serial.println("Unsuccessful connection attempt to:");
    Serial.println(ssid);
}

// ##################################################################
// on wifi disconnect
// ##################################################################
void IRAM_ATTR onWiFiEvent(WiFiEvent_t event)
{
    if (event == SYSTEM_EVENT_STA_DISCONNECTED)
    {
        WifiConnect(ssid, password);
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(100);
        }
        else
        {
            Serial.println("Connected to the WiFi network");
            Serial.print("Local ESP32 IP: ");
            Serial.println(WiFi.localIP());
            return;
        }
    }
}

// ##################################################################
// Get ID list request
// ##################################################################
void sendGETList()
{
    Serial.println("Send HTTP GET request");
    // Send HTTP GET request
    HTTPClient http;
    http.begin(url_client + "/GETLIST");
    http.addHeader("X-Custom-ID", String(chipid));
    // Serial.println(url_client);
    int httpCode = http.GET();

    if (httpCode == 200)
    {

        String res = http.getString();

        // Parse JSON response
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, res);
        if (error)
        {
            // Failed to parse JSON
            Serial.println("Failed to parse JSON!");
            return;
        }

        for (JsonVariant row : doc.as<JsonArray>())
        {
            ids.insert(row["person_id"].as<int>());
            // int person_id = row["person_id"].as<int>();
            // Serial.print("\nID Number: ");
            // Serial.println(person_id);
        }
        Serial.println("GET was successful");
    }
    else if (httpCode == 404)
    {
        // Resource not found on server
        Serial.println("Error 404: resource not found");
    }
    else if (httpCode == 500)
    {
        // Server error
        Serial.println("Error 500: internal server error");
    }
    else
    {
        // Other error occurred
        Serial.println("Error sending GET request");
    }

    http.end();
}

// ##################################################################
// POST request
// ##################################################################
void sendPOSTRequest()
{
    Serial.println("Send HTTP POST request");
    // Send HTTP POST request
    HTTPClient http;
    http.begin(url_client + "/ESP32POSTLOG");
    http.addHeader("X-Custom-ID", String(chipid)); // Add custom ID header

    String postData = "param1=value1&param2=value2"; // Replace with your POST data
    http.POST(postData);

    http.end();
}