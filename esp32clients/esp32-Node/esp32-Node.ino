#include "painlessMesh.h"

#define MESH_PREFIX     "KiwiBotTracking"
#define MESH_PASSWORD   "GoBlugolds"
#define MESH_PORT       5555

Scheduler userScheduler;
painlessMesh mesh;

// Sensor simulation
float temperature = 22.5;
float humidity = 65.0;
uint32_t sensorId = 1001;
unsigned long lastHeartbeatMs = 0;

// Task to send sensor data every 30 seconds
Task taskSendSensor(30000, TASK_FOREVER, [](){
    // Simulate sensor readings with some variation
    temperature += random(-10, 10) / 10.0;
    humidity += random(-50, 50) / 10.0;
    
    // Keep values in reasonable ranges
    temperature = constrain(temperature, 15.0, 35.0);
    humidity = constrain(humidity, 30.0, 90.0);
    
    // Create JSON message
    String msg = "{";
    msg += "\"type\":\"sensor\",";
    msg += "\"nodeId\":" + String(mesh.getNodeId()) + ",";
    msg += "\"sensorId\":" + String(sensorId) + ",";
    msg += "\"temperature\":" + String(temperature, 1) + ",";
    msg += "\"humidity\":" + String(humidity, 1) + ",";
    msg += "\"timestamp\":" + String(mesh.getNodeTime());
    msg += "}";
    
    mesh.sendBroadcast(msg);
    Serial.printf("Sent sensor data: T=%.1f degC, H=%.1f%%\n", temperature, humidity);
});

void receivedCallback(uint32_t from, String &msg) {
    Serial.printf("Sensor Node: Received from %u: %s\n", from, msg.c_str());
    
    // Sensor nodes can respond to commands
    if (msg.indexOf("\"command\":\"read_sensor\"") > 0) {
        // Force immediate sensor reading
        taskSendSensor.forceNextIteration();
        Serial.println("Forced sensor reading requested");
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
    
    mesh.setDebugMsgTypes(ERROR);
    mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onChangedConnections(&changedConnectionCallback);
    
    userScheduler.addTask(taskSendSensor);
    taskSendSensor.enable();
    
    Serial.printf("Sensor Node initialized. Node ID: %u\n", mesh.getNodeId());
}

void loop() {
    mesh.update();

    if (millis() - lastHeartbeatMs >= 1000) {
        lastHeartbeatMs = millis();
        Serial.printf("Heartbeat: uptime=%lu ms, nodeId=%u\n", lastHeartbeatMs, mesh.getNodeId());
    }
}