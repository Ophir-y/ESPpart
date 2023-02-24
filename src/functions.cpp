#include "functions.h"
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

std::set<long> check_ids;
std::set<long> ids;

const char *ssid = "project_wifi";
const char *password = "12345678";
// const char *ssid = "OphirBZK";
// const char *password = "noop2802";
// const char* ssid = "Rothsl";
// const char* password = "Bana&nitzan";

uint64_t chipid = ESP.getEfuseMac();
String port = "1231";

// String url_client = "http://192.168.1.69:" + port;
String url_client = "http://192.168.31.69:" + port;
// Global strings for file commands
String Permitted_ID_LIST_file = "/list_data.txt";
String LOG_file = "/log_data.txt";
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
        // Parse JSON response max of about 1000 people per door
        DynamicJsonDocument doc(32768);
        DeserializationError error = deserializeJson(doc, res);
        if (error)
        {
            // Failed to parse JSON
            Serial.println("Failed to parse JSON!");
            return;
        }
        for (JsonVariant row : doc.as<JsonArray>())
        {
            check_ids.insert(row["person_id"].as<long>());
        }
        // only open and write a new file if the recived list is different from list on board.
        if (ids != check_ids)
        {
            ids = check_ids;
            // Save response to file
            File file = SPIFFS.open(Permitted_ID_LIST_file, FILE_WRITE);
            if (!file)
            {
                Serial.println("Error opening file");
            }
            else
            {
                file.println(res);
                file.close();
                Serial.println("File saved");
                Serial.println("GET was successful");
            }
        }
        else
        {
            Serial.println("No change in list");
        }
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

// ##################################################################
// load data from file to the ids set
// ##################################################################

void LoadFileToIDSet()
{
    File file = SPIFFS.open(Permitted_ID_LIST_file, FILE_READ);
    if (file)
    {
        DynamicJsonDocument doc(32768);

        DeserializationError error = deserializeJson(doc, file);

        if (error)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return;
        }
        for (JsonVariant item : doc.as<JsonArray>())
        {
            ids.insert(item["person_id"].as<long>());
        }
        file.close();
    }
}

// ##################################################################
// print the time
// ##################################################################

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
// ##################################################################
// ask the server for time
// ##################################################################

void SendGetTime()
{
    Serial.println("Send HTTP GET request");
    // Send HTTP GET request
    HTTPClient http;
    http.begin(url_client + "/time");
    http.addHeader("X-Custom-ID", String(chipid));
    // Serial.println(url_client);
    int httpCode = http.GET();

    if (httpCode == 200)
    {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        long timestamp = doc["timestamp"];
        Serial.println("Received timestamp: " + String(timestamp));

        // Set the ESP32's internal clock to the received time
        struct timeval tv = {.tv_sec = timestamp};
        settimeofday(&tv, NULL);
    }
    else
    {
        Serial.print("HTTP time GET request failed, error: ");
        Serial.println(httpCode);
    }
    http.end();
}