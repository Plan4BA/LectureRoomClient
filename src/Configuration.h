
#include <EEPROM.h>
#include <Arduino.h>


typedef struct LectureConfig{
    char wifiAP[50];
    char wifiPW[30];
    char room[10];
    char roomPW[30];
} LectureConfig;

bool enableConfigurationMode = false;
LectureConfig config;

void handleInterrupt()
{
    enableConfigurationMode = true;
}

LectureConfig getConfig()
{
    return config;
}
LectureConfig loadConfig() 
{
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

String waitForInput() 
{
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
    LectureConfig localConfig = loadConfig();
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
          //reinitWlan();
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