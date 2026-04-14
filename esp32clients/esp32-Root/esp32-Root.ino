#include <WiFi.h>
#include <Regexp.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>
#include <list>

#define MESH_PREFIX     "KiwiBotTracking"
#define MESH_PASSWORD   "GoBlugolds"
#define ROUTER_SSID     "Aiden-iPhone"
#define ROUTER_PASSWORD "aidenleee"

String HOSTNAME="kiwibots.myputer.org"
String PATH=""

Scheduler userScheduler;
painlessMesh mesh;
unsigned long lastHeartbeatMs = 0;
const unsigned long heartbeatIntervalMs = 5000;

const char* ssidPattern = "Aiden*";  // regex for SSIDs
const unsigned long scanIntervalMs = 3000;

void receivedCallback(uint32_t from, String& msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void printConnections();

String config;

void getConfig() {
    // Implement HTTP POST to cloud service here
    HTTPClient http;
    http.begin(HOSTNAME + PATH + "/config"); //HTTP
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            config = payload;
            Serial.println(payload);
        } else {
        // HTTP header has been sent and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}

Task updateConfig(30000, TASK_FOREVER, [](){
    // Periodically fetch config from cloud service
    Serial.println("Fetching config from cloud...");
    getConfig();
});



void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\nStarting bridge initialization...");

        WiFi.mode(WIFI_STA);
        WiFi.begin(ROUTER_SSID, ROUTER_PASSWORD);

        uint8_t routerChannel = 1;
        unsigned long startMs = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - startMs) < 10000) {
            delay(200);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            routerChannel = WiFi.channel();
            Serial.printf("\nRouter connected, channel=%u, ip=%s\n", routerChannel,
                                        WiFi.localIP().toString().c_str());
            WiFi.disconnect();
            delay(200);
        } else {
            Serial.println("\nRouter connect timeout, defaulting mesh channel to 1");
        }
    
    mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    
        mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, 5555, WIFI_AP_STA,
                            routerChannel);
        mesh.stationManual(ROUTER_SSID, ROUTER_PASSWORD, 0);
        mesh.setRoot(true);
        mesh.setContainsRoot(true);

    userScheduler.addTask(updateConfig);
    updateConfig.enable();
    
    Serial.println("Bridge setup complete!");
}

void loop() {
    mesh.update();

    if (millis() - lastHeartbeatMs >= heartbeatIntervalMs) {
        lastHeartbeatMs = millis();
        SimpleList<uint32_t> nodes = mesh.getNodeList();
        Serial.printf("[HB] root=%u nodes=%u freeHeap=%u\n",
                    mesh.getNodeId(),
                    nodes.size(),
                    ESP.getFreeHeap());
    }
}


void receivedCallback(uint32_t from, String& msg) {
  // Forward mesh data to Internet services (MQTT, HTTP, etc.)
    Serial.printf("Received from %u: %s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("[MESH] Node joined: %u\n", nodeId);
    printConnections();
}

void changedConnectionCallback() {
    Serial.println("[MESH] Topology changed");
    printConnections();
}

void printConnections() {
    SimpleList<uint32_t> nodes = mesh.getNodeList();
    Serial.printf("[MESH] Connected nodes (%u): ", nodes.size());

    bool first = true;
    for (uint32_t id : nodes) {
        if (!first) {
            Serial.print(", ");
        }
        Serial.print(id);
        first = false;
    }
    Serial.println();
}