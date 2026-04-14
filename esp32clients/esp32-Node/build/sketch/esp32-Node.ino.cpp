#include <Arduino.h>
#line 1 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
/*
 * wCTF Random SSID Generator for ESP32
 *
 * On each boot, generates a random 3-character suffix (A-Z, 0-9)
 * and brings up a Wi-Fi access point named wCTF-XXX.
 *
 * Board: ESP32 (any variant)
 * Framework: Arduino
 */

#include <WiFi.h>

// Characters allowed in the suffix: uppercase letters + digits
static const char CHARSET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const int  CHARSET_LEN = sizeof(CHARSET) - 1; // exclude null terminator

// AP password — set to "" for an open network
static const char *AP_PASSWORD = "";

#line 20 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void setup();
#line 60 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void loop();
#line 20 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void setup() {
  Serial.begin(115200);
  delay(500);

  // Seed the RNG from the ESP32's hardware entropy source
  // esp_random() pulls from the hardware RNG and is available without any extra includes
  randomSeed(esp_random());

  // Build the suffix
  char suffix[4]; // 3 chars + null terminator
  for (int i = 0; i < 3; i++) {
    suffix[i] = CHARSET[random(CHARSET_LEN)];
  }
  suffix[3] = '\0';

  // Assemble the full SSID
  char ssid[16]; // "wCTF-" (5) + 3 chars + null = 9 bytes, 16 is safe
  snprintf(ssid, sizeof(ssid), "wCTF-%s", suffix);

  Serial.printf("[*] Starting AP with SSID: %s\n", ssid);

  // Configure and start the access point
  WiFi.mode(WIFI_AP);

  bool ok;
  if (strlen(AP_PASSWORD) == 0) {
    ok = WiFi.softAP(ssid); // open network
  } else {
    ok = WiFi.softAP(ssid, AP_PASSWORD);
  }

  if (ok) {
    Serial.printf("[+] AP started successfully.\n");
    Serial.printf("[+] SSID    : %s\n", ssid);
    Serial.printf("[+] IP addr : %s\n", WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("[-] Failed to start AP. Check your ESP32 Wi-Fi hardware.");
  }
}

void loop() {
  // Nothing to do — AP runs in the background managed by the SDK
  delay(1000);
}
