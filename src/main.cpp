#include <Arduino.h>
#include <Configuration.h>
#include <WLAN.h>

//e-ink display

/*ConfigurationMode*/

GxIO_Class io(SPI, D1, D3, D4);
    GxEPD_Class display(io, D4,D2);
void setup() {

		//Serial.begin(115200);
		display.init(115200); // enable diagnostic output on Serial
    //show(display, "09:00", "HalloWelt", "Brunner");

    //Interrupt für ConfigurationMode
		pinMode(D6, INPUT);
    attachInterrupt(digitalPinToInterrupt(D6), handleInterrupt, FALLING);

		loadConfig();

    Serial.begin(115200);
    for (uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
		initWlan(getConfig().wifiAP, getConfig().wifiPW);
}

void loop() {
		if ((WiFiMulti.run() == WL_CONNECTED)) {
        WiFiClient client;
        Serial.printf("do in Wifi\n");
        doWithWifi(client, display, getConfig().room, getConfig().roomPW);
    }
    
    if(enableConfigurationMode)
		{
			enterConfigurationMode();
		}
    delay(1000);
		
}