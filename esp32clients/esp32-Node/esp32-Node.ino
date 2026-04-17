#include "painlessMesh.h"
#include <WiFi.h>
#include <Regexp.h>
#include <ArduinoJson.h>


#define MESH_PREFIX     "KiwiBotTracking"
#define MESH_PASSWORD   "GoBlugolds"
#define MESH_PORT       5555

Scheduler userScheduler;
painlessMesh mesh;
uint32_t nodeId = 0; // Unique ID for this node, preset when flashed. Will be written on case. Used to identify the node in the network when sending messages to the root
uint32_t rootNodeId = 0; // Learned from inbound root control message

// Configurable runtime variables
unsigned long scanIntervalMs = 10000;  // Scan task interval (ms)
char ssidPatternBuf[64] = "Aiden.*";  // Pattern for SSIDs: make writable
int minRssiDbm = -100;  // Min RSSI threshold for reporting (-100 to -20)

unsigned long lastHeartbeatMs = 0;

// Task to send sensor data every scanIntervalMs milliseconds
Task taskSendSensor(scanIntervalMs, TASK_FOREVER, [](){
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
    MatchState ms;
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        char ssidBuf[33];
        ssid.toCharArray(ssidBuf, sizeof(ssidBuf));

        ms.Target(ssidBuf);
        if (ms.Match(ssidPatternBuf) > 0) {
            int rssi = WiFi.RSSI(i);
            uint8_t* bssid = WiFi.BSSID(i);

            char macStr[18];
            sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                    bssid[0], bssid[1], bssid[2],
                    bssid[3], bssid[4], bssid[5]);

            Serial.printf("MATCHED: SSID=%s, MAC=%s, RSSI=%d\n",
                          ssid.c_str(), macStr, rssi);

            JsonDocument doc;
            doc["node_id"] = nodeId;
            doc["kiwibot_ssid"] = ssid.c_str();
            doc["rssi"] = rssi;
            doc["kiwibot_mac"] = macStr;
            doc["timestamp"] = mesh.getNodeTime();

            String payload;
            serializeJson(doc, payload);
            if (rootNodeId != 0) {
                mesh.sendSingle(rootNodeId, payload);
            } else {
                // Fallback while root is unknown; root can still receive and command back.
                mesh.sendBroadcast(payload);
            }
        }
    }
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

        if (cfg["scanIntervalMs"].is<unsigned long>()) {
            unsigned long v = cfg["scanIntervalMs"].as<unsigned long>();
            if (v < 1000 || v > 60000) {
                Serial.printf("Scan interval %lu out of range [1000, 60000]. Rejected.\n", v);
                valid = false;
            } else {
                newScanInterval = v;
            }
        }

        if (cfg["ssidPattern"].is<const char*>()) {
            const char* p = cfg["ssidPattern"];
            if (strlen(p) >= sizeof(newPattern)) {
                Serial.printf("SSID pattern too long. Rejected.\n");
                valid = false;
            } else {
                strlcpy(newPattern, p, sizeof(newPattern));
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
}

void changedConnectionCallback() {
    Serial.printf("Sensor Node: Changed connections. Nodes: %s\n", 
                mesh.subConnectionJson().c_str());
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Sensor Node Starting ===");

    WiFi.mode(WIFI_STA);
    
    mesh.setDebugMsgTypes(ERROR);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    
    userScheduler.addTask(taskSendSensor);
    taskSendSensor.enable();

    nodeId = mesh.getNodeId();
    
    Serial.printf("Sensor Node initialized. Node ID: %u\n", mesh.getNodeId());
}

void loop() {
    mesh.update();

    if (millis() - lastHeartbeatMs >= 1000) {
        lastHeartbeatMs = millis();
        Serial.printf("Heartbeat: uptime=%lu ms, nodeId=%u\n", lastHeartbeatMs, mesh.getNodeId());
    }
}