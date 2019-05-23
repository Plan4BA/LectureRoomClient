

#include <GxGDEW027W3/GxGDEW027W3.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>
#include <Global.h>


// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <Icons.h>

ESP8266WiFiMulti WiFiMulti;//WIFI

void initWlan(char *ap, char *pw) 
{
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ap, pw);
}

void reinitWlan(char *ap, char *pw)
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ap, pw);
    Serial.printf("WLAN Konfig wurde neugeladen\n");
}


void show(GxEPD_Class &display, const char now[50][3], const char next[50][3])
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.setRotation(1);
    display.setCursor(0, 0);
    if(now != NULL) {
        int size = sizeof(now) / sizeof(now[0]);
        for(int i = 0 ; i < size ; i++)  {
            display.println(now[i]);
        }
    }

    if(next != NULL) {
        int size = sizeof(next) / sizeof(next[0]);
        for(int i = 0 ; i < size ; i++)  {
            display.println(next[i]);
        }
    }
    
	delay(3000);
    display.update();
    Serial.printf("Update\n");
    delay(3000);
}

void showDisplay(GxEPD_Class &display, const char *payload)
{
                StaticJsonDocument<512> doc;
                DeserializationError error = deserializeJson(doc, payload);
                if (error) {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.c_str());
                    return;
                }
                Serial.println(payload);
                char nowArr[50][3];
                char nextArr[50][3];
                if(!doc["now"].isNull())
                {
                    const char *time = doc["now"]["time"];
                    const char *desc = doc["now"]["desc"];
                    const char *instructor = doc["now"]["instructor"];
                    Serial.printf("(%s, %s, %s)", time, desc, instructor);
                    strcpy(nowArr[0], time);
                    strcpy(nowArr[1], desc);
                    strcpy(nowArr[2], instructor);
                }
                if(!doc["next"].isNull())
                {
                    const char *time = doc["next"]["time"];
                    const char *desc = doc["next"]["desc"];
                    const char *instructor = doc["next"]["instructor"];
                    Serial.printf("(%s, %s, %s)", time, desc, instructor);
                    strcpy(nextArr[0], time);
                    strcpy(nextArr[1], desc);
                    strcpy(nextArr[2], instructor);
                }
                show(display,nowArr, nextArr);
}

void doWithWifi(WiFiClient wiFiClient, GxEPD_Class &display, char *room, char *roompw)
{
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    if (http.begin(wiFiClient, "http://192.168.43.51:8080/meeting")) {  // HTTP
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        char* auth = (char*) malloc(sizeof(char) * (strlen(room) + strlen(roompw) + 1));
        strcpy(auth, config.room);
        strcat(auth, ":");
        strcat(auth, config.roomPW);
        String base64 = base64::encode(auth);
        http.addHeader("Authorization", "Basic " + base64);
        int httpCode = http.GET();
        Serial.printf("HttCode: %d\n", httpCode);
        // httpCode will be negative on error
        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                const char *payload = http.getString().c_str();
                showDisplay(display, payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    } else {
        Serial.printf("[HTTP} Unable to connect\n");
    }
}
