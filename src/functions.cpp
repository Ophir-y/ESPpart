#include "functions.h"
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// const char *ssid = "project_wifi";
// const char *password = "12345678";
const char *ssid = "OphirBZK";
const char *password = "noop2802";
// const char* ssid = "Rothsl";
// const char* password = "Bana&nitzan";

const char *id = "ESP-Ophir";
String port = "1231";

String url_client = "http://192.168.1.69:" + port + "/Client";

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
// Get request
// ##################################################################
void sendGETRequest()
{
    Serial.println("Send HTTP GET request");
    // Send HTTP GET request
    HTTPClient http;
    http.begin(url_client);
    http.addHeader("X-Custom-ID", id);
    // Serial.println(url_client);
    int httpCode = http.GET();

    if (httpCode == 200)
    {
        DynamicJsonDocument jsonBuffer(2048);
        deserializeJson(jsonBuffer, http.getStream());

        // Access the properties of the JSON object
        String first_name = jsonBuffer["first_name"];
        String last_name = jsonBuffer["last_name"];
        String message = first_name + " " + last_name;
        int ID = jsonBuffer["person_id"];
        Serial.println("Message: ");
        Serial.println(message);
        Serial.println("Value: ");
        Serial.println(ID);

        // Request was successful
        String payload = http.getString();
        Serial.println(payload);
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
    http.begin(url_client);
    http.addHeader("X-Custom-ID", id); // Add custom ID header

    String postData = "param1=value1&param2=value2"; // Replace with your POST data
    http.POST(postData);

    http.end();
}