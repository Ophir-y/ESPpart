#include "functions.h"
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// std::set<long> check_ids;
// std::set<long> ids;

// // Define the maximum number of IDs the door can store
// #define MAX_IDS 1000

// Define the array to hold ID data
idData ids[MAX_IDS];
idData check_ids[MAX_IDS];

// Define the number of IDs currently stored
int numIds = 0;

hw_timer_t *watchDogTimer = NULL;

char *key = "q4t7w9z$C&F)J@NcRfUjXn2r5u8x/A%D";
Cipher *cipher = new Cipher(key);

int tocheckid = 0;
int tocheckid_d = 0;

// const char *ssid = "project_wifi";
// const char *password = "12345678";
const char *ssid = "OphirBZK";
const char *password = "noop2802";
// const char* ssid = "Rothsl";
// const char* password = "Bana&nitzan";

uint64_t chipid = ESP.getEfuseMac();
String port = "1231";

String url_client = "http://192.168.1.69:" + port;
// String url_client = "http://192.168.68.108:" + port;
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
// **********************************************************************************
// ##################################################################
// Get ID list request
// ##################################################################

// void sendGETList()
// {
//     Serial.println("Send HTTP GET request");
//     // cipher->setKey(key);
//     // Send HTTP GET request
//     HTTPClient http;
//     http.begin(url_client + "/GETLIST");
//     http.addHeader("X-Custom-ID", String(chipid));
//     // Serial.println(url_client);
//     int httpCode = http.GET();
//     if (httpCode == 200)
//     {
//         String res = http.getString();
//         // String save_res = res;
//         String save_res = cipher->encryptString(res);
//         // Parse JSON response max of about 1000 people per door
//         DynamicJsonDocument doc(32768);
//         DeserializationError error = deserializeJson(doc, res);
//         if (error)
//         {
//             // Failed to parse JSON
//             Serial.println("Failed to parse JSON!");
//             return;
//         }
//         for (JsonVariant row : doc.as<JsonArray>())
//         {
//             check_ids.insert(row["person_id"].as<long>());
//         }
//         // only open and write a new file if the recived list is different from list on board.
//         if (ids != check_ids)
//         {
//             ids = check_ids;
//             // Save response to file
//             File file = SD.open(Permitted_ID_LIST_file, FILE_WRITE);
//             if (!file)
//             {
//                 Serial.println("Error opening file");
//             }
//             else
//             {
//                 file.println(save_res);
//                 file.close();
//                 Serial.println("File saved");
//                 Serial.println("GET was successful");
//             }
//         }
//         else
//         {
//             Serial.println("No change in list");
//         }
//     }
//     else
//     {
//         // Other error occurred
//         Serial.println("Error sending GET request");
//     }
//     http.end();
// }

void sendGETList()
{
    Serial.println("Send HTTP GET request");
    // cipher->setKey(key);
    // Send HTTP GET request
    HTTPClient http;
    http.begin(url_client + "/GETLIST");
    http.addHeader("X-Custom-ID", String(chipid));
    // Serial.println(url_client);
    int httpCode = http.GET();
    int i, check;
    if (httpCode == 200)
    {
        String res = http.getString();
        // String save_res = res;
        String save_res = cipher->encryptString(res);
        // Parse JSON response max of about 1000 people per door
        DynamicJsonDocument doc(32768);
        DeserializationError error = deserializeJson(doc, res);
        if (error)
        {
            // Failed to parse JSON
            Serial.println("Failed to parse JSON!");
            return;
        }
        numIds = 0;
        for (JsonVariant object : doc.as<JsonArray>())
        {
            int id = object["person_id"];
            // convert Time from hh:mm to an Int hh*60+mm
            int startTime = convertTimeToInt(object["start_time"]);
            int endTime = convertTimeToInt(object["end_time"]);

            // Add the ID data to the array
            check_ids[numIds].id = id;
            check_ids[numIds].startTime = startTime;
            check_ids[numIds].endTime = endTime;
            // Increment the number of IDs stored
            numIds++;
            // Check if the maximum number of IDs has been reached
            if (numIds >= MAX_IDS)
            {
                break;
            }
        }
        // only open and write a new file if the recived list is different from list on board.
        for (i = 0; i < numIds; i++)
        {
            // Serial.printf("ids[%d].id is: %d",i,ids[i].id);
            // Serial.print(" ");
            // Serial.printf(" startTime is: %d",ids[i].startTime);
            // Serial.print(" ");
            // Serial.printf(" endTime is: %d",ids[i].endTime);
            // Serial.println(" ");
            if (ids[i].id == check_ids[i].id && ids[i].startTime == check_ids[i].startTime && ids[i].endTime == check_ids[i].endTime)
            {
                check = 0;
            }
            else
            {
                for (i; i < numIds; i++)
                {
                    ids[i].id = check_ids[i].id;
                    ids[i].startTime = check_ids[i].startTime;
                    ids[i].endTime = check_ids[i].endTime;
                    // Serial.printf("ids[%d].id is: %d",i,ids[i].id);
                    // Serial.print(" ");
                    // Serial.printf(" startTime is: %d",ids[i].startTime);
                    // Serial.print(" ");
                    // Serial.printf(" endTime is: %d",ids[i].endTime);
                    // Serial.println(" ");
                }
                check = 1;
            }
        }
        if (check == 1)
        {
            // Save response to file
            File file = SD.open(Permitted_ID_LIST_file, FILE_WRITE);
            if (!file)
            {
                Serial.println("Error opening file");
            }
            else
            {
                file.println(save_res);
                file.close();
                Serial.println("File saved");
                Serial.println("GET was successful");
            }
        }
        else
        {
            Serial.println("No change in list");
            tocheckid++;
            Serial.println(" ");
        }
    }
    else
    {
        // Other error occurred
        Serial.println("Error sending GET request");
    }
    http.end();
}

// **********************************************************************************

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

// void LoadFileToIDSet()
// {
//     File file = SD.open(Permitted_ID_LIST_file, FILE_READ);
//     String decryptBufferfill;
//     if (file)
//     {
//         DynamicJsonDocument doc(32768);
//         DeserializationError error = deserializeJson(doc, file);
//         decryptBufferfill = cipher->decryptString(doc.as<String>());
//         if (error)
//         {
//             Serial.print("deserializeJson() failed: ");
//             Serial.println(error.c_str());
//             return;
//         }
//         for (JsonVariant item : doc.as<JsonArray>())
//         {
//             ids.insert(item["person_id"].as<long>());
//         }
//         file.close();
//     }
// }

void LoadFileToIDSet()
{
    //   // Open the data file
    //   File file = SD.open(Permitted_ID_LIST_file, FILE_READ);
    String decryptfillAsString;
    //   // Check if the file opened successfully
    //   if (file) {
    // Parse the JSON data
    DynamicJsonDocument doc(32768);
    //     decryptfillAsString = cipher->decryptString(file.readString());
    decryptfillAsString = ReadFile(Permitted_ID_LIST_file);

    DeserializationError error = deserializeJson(doc, decryptfillAsString);

    if (error)
    {
        // Failed to parse JSON
        Serial.println("Failed to parse JSON!");
        return;
    }
    // deserializeJson(doc, file);

    // Extract the ID data
    numIds = 0;
    for (JsonVariant value : doc.as<JsonArray>())
    {
        int id = value["person_id"];
        int startTime = value["start_time"];
        int endTime = value["end_time"];

        // Add the ID data to the array
        ids[numIds].id = id;
        ids[numIds].startTime = startTime;
        ids[numIds].endTime = endTime;

        // Increment the number of IDs stored
        numIds++;

        // Check if the maximum number of IDs has been reached
        if (numIds >= MAX_IDS)
        {
            break;
        }
    }

    // // Close the file
    // file.close();
    //   }
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

// *************************************************************************************
// ##################################################################
// check if file exists, if not create one
// input: file name
// ##################################################################
void check_make_file(String file_name)
{
    if (!SD.exists(file_name))
    {
        File file1 = SD.open(file_name, FILE_WRITE);
        if (file1)
        {
            Serial.print("Created");
            Serial.println(file_name);
            file1.close();
        }
        else
        {

            Serial.print("Error creating");
            Serial.println(file_name);
        }
    }
}
// *************************************************************************************

// Check if an ID is approved based on the current time
bool isApproved(int id)
{
    time_t now = time(nullptr);
    struct tm *timeinfo;
    timeinfo = localtime(&now);
    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M", timeinfo);
    // Serial.println(buf);

    int hour = timeinfo->tm_hour;
    int minute = timeinfo->tm_min;
    int currentTime = hour * 60 + minute;
    // Get the current time as a time_t value

    // Loop through the ID data array and check if the ID is approved
    for (int i = 0; i < numIds; i++)
    {

        // Check if the ID matches
        if (ids[i].id == id)
        {

            // Check if the current time is between the start and end times
            if (currentTime >= ids[i].startTime && currentTime <= ids[i].endTime)
            {
                return true;
            }
            //   else {
            //     return false;
            //   }
        }
    }
    // If the ID was not found, return false
    return false;
}

int convertTimeToInt(const char *timeStr)
{
    // copy the time string to a mutable buffer
    char buffer[strlen(timeStr) + 1];
    strcpy(buffer, timeStr);

    // split the time string into separate hours and minutes strings
    char *hoursStr = strtok(buffer, ":");
    char *minutesStr = strtok(NULL, ":");

    // convert the hours and minutes strings to integer values
    int hours = atoi(hoursStr);
    int minutes = atoi(minutesStr);

    // convert the time to an integer value in minutes
    int timeInMinutes = hours * 60 + minutes;

    return timeInMinutes;
}

void saveinlog(String isApproved, int id)
{
    time_t now = time(nullptr);
    struct tm *timeinfo;
    timeinfo = localtime(&now);
    String TIS = asctime(timeinfo);
    String OldLog = ReadFile(LOG_file);
    char str[8];
    sprintf(str, "%d", id);
    String theid = str;
    String yn;
    if (isApproved == "Approved")
    {
        yn = "yes";
    }
    else
    {
        yn = "no";
    }
    String tii = "the id is: ";
    String tti = "the time is: ";
    String IA = "is Approved: ";
    String newLog = OldLog + tii + theid + tti + TIS + IA + yn;
    String save_res = cipher->encryptString(newLog);
    File file = SD.open(LOG_file, FILE_WRITE);
    if (!file)
    {
        Serial.println("Error opening file");
    }
    else
    {
        file.println(save_res);
        file.close();
        Serial.println("File saved");
        Serial.println("GET was successful");
    }
}

String ReadFile(String whichfile)
{
    File file = SD.open(whichfile, FILE_READ);
    String decryptfillAsString;
    // Check if the file opened successfully
    if (file)
    {
        // Parse the JSON data
        decryptfillAsString = cipher->decryptString(file.readString());
        file.close();
        return decryptfillAsString;
    }
    else
    {
        // Failed to parse JSON
        Serial.printf("Failed to get the %s from the sd \n", whichfile);
        file.close();
        return "error";
    }
}
