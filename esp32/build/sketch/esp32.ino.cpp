#include <Arduino.h>
#line 1 "/home/aiden/Documents/GitHub/TriangulationNet/esp32/esp32.ino"
#include <WiFi.h>
#include <Regexp.h>

const char* ssidPattern = "Aiden*";  // regex for SSIDs

#line 6 "/home/aiden/Documents/GitHub/TriangulationNet/esp32/esp32.ino"
void setup();
#line 13 "/home/aiden/Documents/GitHub/TriangulationNet/esp32/esp32.ino"
void loop();
#line 6 "/home/aiden/Documents/GitHub/TriangulationNet/esp32/esp32.ino"
void setup() {
Serial.begin(115200);
WiFi.mode(WIFI_STA);
WiFi.disconnect();
delay(100);
}

void loop() {
    int n = WiFi.scanNetworks(false, true); // don't show hidden, show BSSID
    MatchState ms;

    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        std::string ssidStr = ssid.c_str();

        ms.Target((char*)ssidStr.c_str());
        if (ms.Match(ssidPattern) > 0) {
        int rssi = WiFi.RSSI(i);
        uint8_t* bssid = WiFi.BSSID(i);

        char macStr[18];
        sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                bssid[0], bssid[1], bssid[2],
                bssid[3], bssid[4], bssid[5]);

        Serial.printf("MATCHED: SSID=%s, MAC=%s, RSSI=%d\n",
                        ssid.c_str(), macStr, rssi);
        }
    }

    delay(1000);
}
