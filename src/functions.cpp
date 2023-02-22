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
        File file = SPIFFS.open("/list_data.txt", "r");
        String line = file.readStringUntil('\n');
        if(line==response){
            Serial.println("no new informtion");
            file.close();
            http.end();
            return;
        }
        file.close();
        //  if (SPIFFS.remove("/list_data.txt")) {
        //     Serial.println("File deleted successfully");
        // } else {
        //     Serial.println("Failed to delete file");
        // }

        // Save response to file
        File f = SPIFFS.open("/list_data.txt", "w");
        if (!f) {
            Serial.println("Error opening file");
        } else {
            // f.print(response);
            f.close();
            Serial.println("File saved");
        }

        // // Serial.println("Response: " + response);
        // DynamicJsonDocument doc(1024);
        // eserializeJson(doc, response);
        // // print the names
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
// inputs:
// id - input the string that you recived from RFID reader
// ##################################################################

bool isIdAllowed(String id) {
    // change later, this change is done because of the weird id
    String chek123 = "012145171243";
    String newid = "123123123";
    if ( id == chek123){
         id = newid;
    }
    Serial.print("the id is: ");
    Serial.println(id);
    ///####################################################


    // open file
  File file = SPIFFS.open("/list_data.txt", "r");
  
  // true if file retrival was successful
  if (file) {
    // read file lines until reached empty
    while (file.available()) {
    // read first line
      String line = file.readStringUntil('\n');
      // trim white space
      line.trim();
      //create json 
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, line);
      for (JsonVariant row : doc.as<JsonArray>())
        {
            String person_id = row["person_id"];
            Serial.println(person_id);
             if (person_id == id) {
                file.close();
                return true;
            }
        }  
    }

    file.close();
  }
    return false;
}

// ##################################################################
// comper the ID to the stored IDs
// ##################################################################

void SaveLog(String id, String Access)
{
            // Save response to file
        time_t now = time(NULL);
        String the_accsss_as = id + Access; 
        String the_time_is = ctime(&now);
        File f = SPIFFS.open("/log.txt", "w");
        if (!f) {
            Serial.println("Error opening file");
        } else {
            Serial.println(ctime(&now));
            Serial.println(the_accsss_as+the_time_is);
            f.println(the_accsss_as+the_time_is);
            f.close();
            Serial.println("log saved");
        }
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

    if (httpCode == 200){
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        long timestamp = doc["timestamp"];
        Serial.println("Received timestamp: " + String(timestamp));

        // Set the ESP32's internal clock to the received time
        struct timeval tv = { .tv_sec = timestamp };
        settimeofday(&tv, NULL);
  } else {
    Serial.print("HTTP time GET request failed, error: ");
    Serial.println(httpCode);
  }
    http.end();
}
