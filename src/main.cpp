#include <Arduino.h>
#include <Configuration.h>
#include <WLAN.h>

//e-ink display

/*ConfigurationMode*/
/*
Pinbelegung E-paper display
Busy - D2
RST - D4
DC - D3
CS - D1
CLK - D5
DIN - D7
*/
GxIO_Class io(SPI, D1, D3, D4);
GxEPD_Class display(io, D4,D2);

void showStartup(GxEPD_Class &display, const char *message, int leftPadding)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.setRotation(1);
    display.drawBitmap(plan4ba, 52, 0, 160, 160, GxEPD_BLACK);
    display.setCursor(leftPadding, 160);
    display.printf(message);
	  delay(3000);
    display.update();
}

void showError(GxEPD_Class &display)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);
    display.setRotation(1);
    display.setCursor(0, 0);
    display.drawBitmap(fine, 47, 0, 176, 170, GxEPD_BLACK);
	  delay(3000);
    display.update();
}
void setup() {


		Serial.begin(115200);
		display.init(115200); // enable diagnostic output on Serial

    loadConfig();
    char *room = "Raum ";
    strcat(room, getConfig().room);
    showStartup(display, room, 90);

    //Interrupt für ConfigurationMode
		pinMode(D6, INPUT);
    attachInterrupt(digitalPinToInterrupt(D6), handleInterrupt, FALLING);

    Serial.begin(115200);
    for (uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

		initWlan(getConfig().wifiAP, getConfig().wifiPW);
    Serial.println("ESP8266 in sleepmode");

    
    
    //ESP.deepSleep(10 * 1000000);
}

void loop()
{
  if ((WiFiMulti.run() == WL_CONNECTED)) {
        WiFiClient client;
        Serial.printf("do in Wifi\n");
        int status = doWithWifi(client, display, getConfig().room, getConfig().roomPW);
        if(status == -1)
          showError(display);
    }
    delay(5000);
    if(enableConfigurationMode)
		{
      showStartup(display, "ConfigurationMode", 30);
			enterConfigurationMode();
		}
}