#include "functions.h"
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// const char *ssid = "project_wifi";
// const char *password = "12345678";

// const char *ssid = "OphirBZK";
// const char *password = "noop2802";

const char* ssid = "Rothsl";
const char* password = "Bana&nitzan";

uint64_t chipid = ESP.getEfuseMac();
String port = "1231";

String url_client = "http://192.168.68.104:" + port;
// String url_client = "http://192.168.1.69:" + port;

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

    if (httpCode == 200){
        String response = http.getString();
        Serial.println("Response: " + response);

        // Save response to file
        File f = SPIFFS.open("/list_data.txt", "w");
        if (!f) {
            Serial.println("Error opening file");
        } else {
            f.println(response);
            f.close();
            Serial.println("File saved");
        }

        // Serial.println("these are the names and detailes of the students allowed to enter thi room:");
        // for (JsonVariant row : doc.as<JsonArray>())
        // {
        //     int person_id = row["person_id"].as<int>();
        //     Serial.print("\nID Number: ");
        //     Serial.println(person_id);
        // }
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

// ##################################################################
// comper the ID to the stored IDs
// ##################################################################

bool isIdAllowed(String id) {
  File file = SPIFFS.open("/list_data.txt", "r");
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();

      if (line == id) {
        file.close();
        return true;
      }
    }

    file.close();
  }
    return false;
}

// ##################################################################
// comper the ID to the stored IDs
// ##################################################################

bool isIdAllowedOnLI(String id)
{
    Serial.println("Send HTTP GET request");
    // Send HTTP GET request
    HTTPClient http;
    http.begin(url_client + "/GETLIST");
    http.addHeader("X-Custom-ID", String(chipid));
    // Serial.println(url_client);
    int httpCode = http.GET();

    if (httpCode == 200){
       String response = http.getString();
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
// print the time
// ##################################################################

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}