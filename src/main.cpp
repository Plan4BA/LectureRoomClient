#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <base64.h>
#include <EEPROM.h>
#include <LiquidCrystal.h> //LCD-Bibliothek laden
#include <ArduinoJson.h>
LiquidCrystal lcd(D8, D7, D4, D3, D2, D1);

ESP8266WiFiMulti WiFiMulti;

typedef struct {
    char wifiAP[50];
    char wifiPW[30];
    char room[10];
    char roomPW[30];
} EEPROMConfig;

bool enableConfigurationMode = false;

EEPROMConfig config;

EEPROMConfig loadConfig() {
    EEPROM.begin(4095);
    EEPROM.get(512,config);
    EEPROM.end();
    Serial.printf("Load Configuration\n");
    Serial.printf("AP: [%s]\n", config.wifiAP);
    Serial.printf("PW: [%s]\n", config.wifiPW);
    Serial.printf("Room: [%s]\n", config.room);
    Serial.printf("RoomPW: [%s]\n", config.roomPW);
    return config;
}

void handleInterrupt()
{
    enableConfigurationMode = true;
}

void setup() {

    pinMode(D6, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(D6), handleInterrupt, RISING);
    Serial.begin(115200);
    Serial.printf("Test\n");

    for (uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    loadConfig();

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(config.wifiAP, config.wifiPW);
    lcd.begin(16, 2);
}

void doWithWifi(WiFiClient wiFiClient)
{
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    if (http.begin(wiFiClient, "http://192.168.43.51:8080/meeting")) {  // HTTP
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        char* auth = (char*) malloc(sizeof(char) * (strlen(config.room) + strlen(config.roomPW) + 1));
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
                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, payload);
                if (error) {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.c_str());
                    return;
                }
                Serial.println(payload);
                const char *now = doc["now"];
                const char *next = doc["next"];

                lcd.setCursor(0, 0);
                lcd.printf("%s", now);
                lcd.setCursor(0, 1);
                lcd.printf("%s", next);
            }
        } else {
            lcd.setCursor(0, 0);
            lcd.printf("Fehler!");
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    } else {
        Serial.printf("[HTTP} Unable to connect\n");
        lcd.setCursor(0, 0);
        lcd.printf("Fehler!");
    }
}

String waitForInput() {
    String input = Serial.readStringUntil('\n');
    while(input[0] == '\0')
    {
        delay(500);
        input = Serial.readStringUntil('\n');
    }
    input.replace('\r', '\0');
    return input;
}

void enterConfigurationMode()
{
    EEPROMConfig localConfig = loadConfig();
    bool inConfiguration = true;
    do {
        Serial.println("Welche Konfiguration möchten Sie anpassen:\n"
                       "0 - WLAN SSID\n"
                       "1 - WLAN Passwort\n"
                       "2 - Raumnummer\n"
                       "3 - Backend-Login\n"
                       "4 - Konfiguration anzeigen\n"
                       "5 - WLAN-Konfiguration neuladen\n");
        String input = waitForInput();


        if (input.toInt() == 0) {
            Serial.println("\tBitte geben Sie die WLAN SSID an");
            String ssid = waitForInput();
            strcpy(localConfig.wifiAP, ssid.c_str());
            Serial.println("\tSSID wurde gespeichert");
        } else if (input.toInt() == 1) {
            Serial.println("\tBitte geben Sie das WLAN-Passwort an");
            String pw = waitForInput();
            strcpy(localConfig.wifiPW, pw.c_str());
            Serial.println("\tPasswort wurde gespeichert");
        } else if (input.toInt() == 2) {
            Serial.println("\tBitte geben Sie die Raumnummer an");
            String room = waitForInput();
            strcpy(localConfig.room, room.c_str());
            Serial.println("\tRaum wurde gespeichert");
        } else if (input.toInt() == 3) {
            Serial.println("\tBitte geben Sie das Backend Login an");
            String login = waitForInput();
            strcpy(localConfig.roomPW, login.c_str());
            Serial.println("\tBackend Login wurde gespeichert");
        } else if (input.toInt() == 4) {
            loadConfig();
            enableConfigurationMode = false;
            return;
        } else if(input.toInt() == 5) {
          WiFi.disconnect(true);
          WiFi.mode(WIFI_STA);
          WiFiMulti.addAP(config.wifiAP, config.wifiPW);
          Serial.printf("WLAN Konfig wurde neugeladen\n");
          return;
        } else {
            Serial.printf("Konfiguration nicht gefunden [%s]\n", input.c_str());
        }
        Serial.println("Moechten Sie eine weitere Konfigration aendern?[ja - 1 | nein - 0]");
        String exit = waitForInput();
        if(exit.toInt() == 0)
            inConfiguration = false;
    } while(inConfiguration);
    Serial.printf("Folgende Konfiguration wird gespeichert:\n");
    Serial.printf("AP: %s\n", localConfig.wifiAP);
    Serial.printf("PW: %s\n", localConfig.wifiPW);
    Serial.printf("Room: %s\n", localConfig.room);
    Serial.printf("RoomPW: %s\n", localConfig.roomPW);
    Serial.println("Speichern?[ja - 1 | nein - 0]");
    String save = waitForInput();
    if(save.toInt() == 1){
        //clear EEPROM
        EEPROM.begin(4095);
        for (int i = 0; i < sizeof(localConfig) ; i++) {
            EEPROM.write(512 + i, 0);
        }
        delay(200);
        EEPROM.commit();
        EEPROM.end();

        EEPROM.begin(4095);
        EEPROM.put(512, localConfig);
        delay(200);
        EEPROM.commit();
        EEPROM.end();
        Serial.printf("Konfiguration wurde im EEPROM gespeichert\n");

    }

    enableConfigurationMode = false;
}
int a = 0;
void loop() {

    // wait for WiFi connection
    a++;

    if ((WiFiMulti.run() == WL_CONNECTED)) {
        WiFiClient client;
        Serial.printf("do in Wifi\n");
        doWithWifi(client);
    }
    
    if(enableConfigurationMode)
        enterConfigurationMode();
    delay(1000);
}