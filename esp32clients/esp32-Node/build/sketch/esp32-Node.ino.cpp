#include <Arduino.h>
#line 1 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
#include "painlessMesh.h"
#include <WiFi.h>
#include <Regexp.h>
#include <ArduinoJson.h>
#include <esp_rom_sys.h>


#define MESH_PREFIX     "KiwiBotTracking"
#define MESH_PASSWORD   "GoBlugolds"
#define MESH_PORT       5555
#define MESH_CHANNEL    6

Scheduler userScheduler;
painlessMesh mesh;
uint32_t nodeId = 1; 
uint32_t actualNodeId = 1; // Unique ID for this node, preset when flashed. Will be written on case. Used to identify the node in the network when sending messages to the root
uint32_t rootNodeId = 0; // Learned from inbound root control message
bool meshConnected = false;
volatile bool topologyChanged = false;

// Configurable runtime variables
unsigned long scanIntervalMs = 10000;  // Scan task interval (ms)
char ssidPatternBuf[64] = "kiwi.*";  // Pattern for SSIDs: make writable
int minRssiDbm = -100;  // Min RSSI threshold for reporting (-100 to -20)

unsigned long lastHeartbeatMs = 0;

#line 28 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
static const char * resetReasonToText(esp_reset_reason_t reason);
#line 45 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
bool ssidMatchesPattern(const String& ssid);
#line 53 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
static void stripOuterQuotes(String& value);
#line 139 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void receivedCallback(uint32_t from, String &msg);
#line 215 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void newConnectionCallback(uint32_t nodeId);
#line 220 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void changedConnectionCallback();
#line 224 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void setup();
#line 258 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
void loop();
#line 28 "/home/aiden/Documents/GitHub/TriangulationNet/esp32clients/esp32-Node/esp32-Node.ino"
static const char* resetReasonToText(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_UNKNOWN:   return "unknown";
        case ESP_RST_POWERON:   return "poweron";
        case ESP_RST_EXT:       return "external pin";
        case ESP_RST_SW:        return "software";
        case ESP_RST_PANIC:     return "panic";
        case ESP_RST_INT_WDT:   return "interrupt watchdog";
        case ESP_RST_TASK_WDT:  return "task watchdog";
        case ESP_RST_WDT:       return "other watchdog";
        case ESP_RST_DEEPSLEEP: return "deepsleep wake";
        case ESP_RST_BROWNOUT:  return "brownout";
        case ESP_RST_SDIO:      return "sdio";
        default:                return "unmapped";
    }
}

bool ssidMatchesPattern(const String& ssid) {
    MatchState ms;
    char target[33];
    ssid.toCharArray(target, sizeof(target));
    ms.Target(target);
    return ms.Match(ssidPatternBuf) > 0;
}

static void stripOuterQuotes(String& value) {
    value.trim();
    if (value.length() >= 2) {
        char first = value[0];
        char last = value[value.length() - 1];
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            value = value.substring(1, value.length() - 1);
        }
    }
}

// Task to send sensor data every scanIntervalMs milliseconds
Task taskSendSensor(scanIntervalMs, TASK_FOREVER, [](){
    if (!meshConnected) {
        Serial.println("Scan skipped: mesh not connected");
        return;
    }

    Serial.println("Scanning for WiFi networks...");
    int status = WiFi.scanComplete();
    if (status == WIFI_SCAN_FAILED) {
        // No scan running — start a new async one (non-blocking)
        WiFi.scanNetworks(true, true);
        Serial.println("Scan started...");
        return;
    }

    if (status == WIFI_SCAN_RUNNING) {
        return;
    }

    int n = status;
    int matchedCount = 0;
    if (n <= 0) {
        Serial.println("Scan complete: no networks found");
        WiFi.scanDelete();
        WiFi.scanNetworks(true, true);
        return;
    }

    Serial.printf("Scan complete: found %d networks\n", n);
    for (int i = 0; i < n; i++) {
        delay(0); // Keep watchdog fed while processing larger scans.

        String ssid = WiFi.SSID(i);
        if (ssidMatchesPattern(ssid)) {
            matchedCount++;
            int rssi = WiFi.RSSI(i);
            uint8_t* bssid = WiFi.BSSID(i);
            if (bssid == nullptr) {
                Serial.printf("Skipping AP %d due to null BSSID pointer\n", i);
                continue;
            }

            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                     bssid[0], bssid[1], bssid[2],
                     bssid[3], bssid[4], bssid[5]);

            Serial.printf("MATCHED: SSID=%s, MAC=%s, RSSI=%d\n",
                          ssid.c_str(), macStr, rssi);

            JsonDocument doc;
            doc["id"] = actualNodeId;
            doc["ssid"] = ssid.c_str();
            doc["strength"] = rssi;
            // doc["kiwibot_mac"] = macStr;
            // doc["timestamp"] = mesh.getNodeTime();

            String payload;
            serializeJson(doc, payload);
            if (rootNodeId != 0) {
                mesh.sendSingle(rootNodeId, payload);
                Serial.printf("Sent match to root %u: %s\n", rootNodeId, payload.c_str());
            } else {
                // Fallback while root is unknown; root can still receive and command back.
                mesh.sendBroadcast(payload);
                Serial.printf("Sent match as broadcast: %s\n", payload.c_str());
            }
        }
    }
    Serial.printf("Scan summary: matched %d networks with pattern '%s'\n", matchedCount, ssidPatternBuf);
    WiFi.scanDelete();
    WiFi.scanNetworks(true, true);
});

void receivedCallback(uint32_t from, String &msg) {
    Serial.printf("Sensor Node: Received from %u: %s\n", from, msg.c_str());
    
    // Parse incoming JSON message
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) {
        Serial.printf("JSON parse error: %s\n", err.c_str());
        return;
    }

    const char* type = doc["type"] | "";

    // Handle config_update message
    if (strcmp(type, "config_update") == 0) {
        // Trust model: only accept from established root
        if (rootNodeId != 0 && from != rootNodeId) {
            Serial.printf("Config from untrusted sender %u (root is %u). Rejected.\n", from, rootNodeId);
            return;
        }
        rootNodeId = from;  // Update root on first trusted message

        // Check target node: 0 = broadcast to all, otherwise match nodeId
        uint32_t target = doc["target_node"] | 0;
        if (target != 0 && target != nodeId) {
            Serial.printf("Config target mismatch: %u vs %u. Skipped.\n", target, nodeId);
            return;
        }

        JsonObject cfg = doc["config"].as<JsonObject>();
        if (cfg.isNull()) {
            Serial.println("Config object missing.");
            return;
        }

        // Stage new values with validation
        unsigned long newScanInterval = scanIntervalMs;
        char newPattern[64];
        strlcpy(newPattern, ssidPatternBuf, sizeof(newPattern));
        int newMinRssi = minRssiDbm;

        bool valid = true;

        if (cfg["scan_interval_ms"].is<unsigned long>()) {
            unsigned long v = cfg["scan_interval_ms"].as<unsigned long>();
            if (v < 1000 || v > 60000) {
                Serial.printf("Scan interval %lu out of range [1000, 60000]. Rejected.\n", v);
                valid = false;
            } else {
                newScanInterval = v;
            }
        }

        if (cfg["target_address"].is<const char*>()) {
            String pattern = cfg["target_address"].as<String>();
            stripOuterQuotes(pattern);
            if (pattern.length() >= sizeof(newPattern)) {
                Serial.printf("SSID pattern too long. Rejected.\n");
                valid = false;
            } else {
                strlcpy(newPattern, pattern.c_str(), sizeof(newPattern));
            }
        }

        // Commit all changes
        scanIntervalMs = newScanInterval;
        strlcpy(ssidPatternBuf, newPattern, sizeof(ssidPatternBuf));
        minRssiDbm = newMinRssi;

        // Update scheduler with new interval
        taskSendSensor.setInterval(scanIntervalMs);

        Serial.printf("Config applied: interval=%lu ms, pattern=%s, minRssi=%d\n", newScanInterval, newPattern, newMinRssi);
    }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("Sensor Node: New connection to %u\n", nodeId);
    topologyChanged = true;
}

void changedConnectionCallback() {
    topologyChanged = true;
}

void setup() {
    // ROM print helps verify we entered setup even if Serial logging is unstable.
    esp_rom_printf("\nBOOT: setup entered\n");

    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Sensor Node Starting ===");

    esp_reset_reason_t reason = esp_reset_reason();
    Serial.printf("Reset reason: %d (%s)\n", (int)reason, resetReasonToText(reason));
    Serial.println("setup checkpoint: serial started");

    Serial.println("setup checkpoint: configuring WiFi mode");
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    
    Serial.println("setup checkpoint: mesh init start");
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL);
    Serial.println("setup checkpoint: mesh init complete");
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);

    mesh.setContainsRoot(true);
    
    userScheduler.addTask(taskSendSensor);
    taskSendSensor.enable();

    nodeId = mesh.getNodeId();
    
    Serial.printf("Sensor Node initialized. Node ID: %u\n", mesh.getNodeId());
}

void loop() {
    mesh.update();

    // Handle topology inspection in loop context instead of inside callbacks.
    if (topologyChanged) {
        topologyChanged = false;
        size_t peers = mesh.getNodeList().size();
        meshConnected = peers > 0;
        String topo = mesh.subConnectionJson();
        Serial.printf("Sensor Node: Topology changed. peers=%u nodes=%s\n",
                      (unsigned int)peers,
                      topo.c_str());
    }

    if (millis() - lastHeartbeatMs >= 1000) {
        lastHeartbeatMs = millis();
        Serial.printf("Heartbeat: uptime=%lu ms, nodeId=%u, peers=%u, connected=%s\n",
                      lastHeartbeatMs,
                      mesh.getNodeId(),
                      (unsigned int)mesh.getNodeList().size(),
                      meshConnected ? "yes" : "no");
    }
}
