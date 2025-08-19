/*
 * FlameWorks ESP32-C6 Burner Controller & App Host
 * Hosts the React Native app directly on the ESP32-C6 chip
 * 
 * Features:
 * - Creates WiFi Access Point "FlameWorks-[ID]"
 * - Hosts React Native web app at http://192.168.4.1
 * - Real-time burner control via WebSocket
 * - Multiple burner support with drag-select
 * - Safety features: flame sensing, ignition timeout, fault detection
 * - Works completely offline - no internet required
 * - Battery powered operation
 */

#include <WiFi.h>
#include <AsyncWebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// Network Configuration
const char* ap_ssid_prefix = "FlameWorks-";
const char* ap_password = "flame2024";
const IPAddress local_ip(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

// Servers
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
Preferences preferences;

// Hardware Pins (ESP32-C6 Compatible)
const int PILOT_VALVE_PIN = 4;    // GPIO 4 - Pilot valve relay
const int MAIN_VALVE_PIN = 5;     // GPIO 5 - Main valve relay  
const int IGNITER_PIN = 6;        // GPIO 6 - Igniter control
const int FLAME_SENSOR_PIN = 7;   // GPIO 7 - Flame sensor (digital)
const int STATUS_LED_PIN = 8;     // GPIO 8 - Status LED (built-in)
const int TEMP_SENSOR_PIN = 1;    // GPIO 1 - Temperature sensor (analog)
const int EMERGENCY_STOP_PIN = 9; // GPIO 9 - Emergency stop button

// Burner System Configuration
const int MAX_BURNERS = 8;        // Support up to 8 burners
const int IGNITION_TIMEOUT = 5000; // 5 seconds
const int MAX_RETRY_ATTEMPTS = 3;
const int FLAME_THRESHOLD = 512;   // ADC threshold for flame detection

// Burner States
enum BurnerState {
  BURNER_OFF = 0,
  PILOT_IGNITING = 1,
  PILOT_ON = 2,
  MAIN_IGNITING = 3,
  MAIN_ON = 4,
  FAULT = 5,
  EMERGENCY_STOP = 6
};

// Burner Structure
struct Burner {
  int id;
  BurnerState state;
  bool selected;
  int pilotValvePin;
  int mainValvePin;
  int igniterPin;
  int flameSensorPin;
  unsigned long ignitionStartTime;
  int retryCount;
  int temperature;
  bool faultCondition;
  String lastError;
};

// System State
Burner burners[MAX_BURNERS];
bool masterSwitch = false;
bool emergencyStop = false;
int activeBurners = 1; // Default to single burner
String systemId;
unsigned long lastStatusUpdate = 0;
bool bluetoothEnabled = false;

// Helper function to log state changes
void logStateChange(int burnerIndex, BurnerState oldState, BurnerState newState) {
  String oldStateName = getStateName(oldState);
  String newStateName = getStateName(newState);
  
  Serial.printf("🔄 Burner %d: %s → %s\n", burnerIndex + 1, oldStateName.c_str(), newStateName.c_str());
}

String getStateName(BurnerState state) {
  switch(state) {
    case BURNER_OFF: return "OFF";
    case PILOT_IGNITING: return "PILOT IGNITING";
    case PILOT_ON: return "PILOT ON"; 
    case MAIN_IGNITING: return "MAIN IGNITING";
    case MAIN_ON: return "MAIN ON";
    case BURNER_FAULT: return "FAULT";
    case EMERGENCY_STOP: return "E-STOP";
    default: return "UNKNOWN";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give serial monitor time to connect
  
  Serial.println("\n\n=============================================");
  Serial.println("🔥 FlameWorks Burner Controller - ESP32-C6");
  Serial.println("=============================================");
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());  
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("---------------------------------------------");
  
  Serial.println("📝 Initializing preferences...");
  // Initialize preferences
  preferences.begin("flameworks", false);
  activeBurners = preferences.getInt("burners", 1);
  Serial.printf("✅ Active burners: %d\n", activeBurners);
  
  // Generate system ID
  systemId = "FW" + String(ESP.getEfuseMac(), HEX).substring(6);
  Serial.printf("🆔 System ID: %s\n", systemId.c_str());
  
  Serial.println("📌 Initializing hardware pins...");
  // Initialize hardware pins
  initializeHardware();
  Serial.println("✅ Hardware pins initialized");
  
  Serial.println("🔥 Initializing burner systems...");
  // Initialize burner array
  initializeBurners();
  Serial.println("✅ Burner systems ready");
  
  Serial.println("💾 Mounting SPIFFS file system...");
  // Initialize file system
  if(!SPIFFS.begin(true)){
    Serial.println("❌ SPIFFS Mount Failed");
    return;
  }
  Serial.println("✅ SPIFFS mounted successfully");
  
  Serial.println("📶 Creating WiFi Access Point...");
  // Create WiFi Access Point
  String apName = ap_ssid_prefix + systemId;
  WiFi.softAP(apName.c_str(), ap_password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.printf("✅ WiFi AP created: %s\n", apName.c_str());
  Serial.printf("🔑 Password: %s\n", ap_password);
  Serial.printf("🌐 IP Address: %s\n", local_ip.toString().c_str());
  
  Serial.println("🌐 Starting web server...");
  // Set up web server
  setupWebServer();
  Serial.println("✅ Web server started on port 80");
  
  Serial.println("🔌 Starting WebSocket server...");
  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("✅ WebSocket server started on port 81");
  
  // Start watchdog timer
  esp_task_wdt_init(30, true);
  esp_task_wdt_add(NULL);
  
  Serial.println("\n=============================================");
  Serial.println("🎉 SYSTEM READY!");
  Serial.println("=============================================");
  Serial.printf("📡 Connect to WiFi: %s\n", apName.c_str());
  Serial.printf("🔑 Password: %s\n", ap_password);
  Serial.printf("🌐 Web Interface: http://%s\n", local_ip.toString().c_str());
  Serial.printf("🔥 Active Burners: %d\n", activeBurners);
  Serial.println("=============================================");
  Serial.println("\n📝 SERIAL COMMANDS:");
  Serial.println("   status    - Show system status");
  Serial.println("   test      - Test all relays (LED mode)");
  Serial.println("   ignite X  - Ignite burner X (1-8)");
  Serial.println("   stop X    - Stop burner X (1-8)");
  Serial.println("   emergency - Emergency stop all");
  Serial.println("   reset     - Reset system");
  Serial.println("   help      - Show this help");
  Serial.println("=============================================\n");
  
  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  // Start web server
  server.begin();
  
  // System ready
  digitalWrite(STATUS_LED_PIN, HIGH);
  Serial.println("System Ready - FlameWorks App Hosted on ESP32-C6!");
}

void loop() {
  // Handle serial commands
  handleSerialCommands();
  
  // Main system operations
  webSocket.loop();
  updateBurnerSystem();
  handleEmergencyStop();
  broadcastStatus();
  
  // Feed watchdog
  esp_task_wdt_reset();
  
  delay(50); // 20Hz update rate
}

void initializeHardware() {
  // Configure pins
  pinMode(PILOT_VALVE_PIN, OUTPUT);
  pinMode(MAIN_VALVE_PIN, OUTPUT);
  pinMode(IGNITER_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
  
  // Initialize all outputs to safe state (OFF)
  digitalWrite(PILOT_VALVE_PIN, LOW);
  digitalWrite(MAIN_VALVE_PIN, LOW);
  digitalWrite(IGNITER_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  Serial.println("Hardware initialized - SAFE STATE");
}

void initializeBurners() {
  for (int i = 0; i < MAX_BURNERS; i++) {
    burners[i].id = i + 1;
    burners[i].state = BURNER_OFF;
    burners[i].selected = false;
    burners[i].pilotValvePin = PILOT_VALVE_PIN + (i * 3);  // Expand for multiple burners
    burners[i].mainValvePin = MAIN_VALVE_PIN + (i * 3);
    burners[i].igniterPin = IGNITER_PIN + (i * 3);
    burners[i].flameSensorPin = FLAME_SENSOR_PIN;
    burners[i].ignitionStartTime = 0;
    burners[i].retryCount = 0;
    burners[i].temperature = 20; // Room temperature
    burners[i].faultCondition = false;
    burners[i].lastError = "";
  }
  
  Serial.print("Initialized ");
  Serial.print(activeBurners);
  Serial.println(" burner(s)");
}

void setupWebServer() {
  // Serve main React Native app
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  // Serve static files (JS, CSS, images)
  server.serveStatic("/", SPIFFS, "/");
  
  // API endpoints for burner control
  server.on("/api/burners", HTTP_GET, handleGetBurners);
  server.on("/api/burner/ignite", HTTP_POST, handleIgniteBurner);
  server.on("/api/burner/shutdown", HTTP_POST, handleShutdownBurner);
  server.on("/api/master", HTTP_POST, handleMasterSwitch);
  server.on("/api/select", HTTP_POST, handleBurnerSelection);
  server.on("/api/system", HTTP_GET, handleSystemStatus);
  server.on("/api/emergency", HTTP_POST, handleEmergencyStop);
  
  // Handle not found
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "FlameWorks: Page not found");
  });
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Client disconnected\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected: %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      
      // Send current system status to new client
      sendSystemStatusToClient(num);
      break;
    }
    
    case WStype_TEXT: {
      String command = String((char*)payload);
      Serial.printf("[%u] Command: %s\n", num, command.c_str());
      
      // Parse and execute WebSocket commands
      executeWebSocketCommand(command);
      break;
    }
    
    default:
      break;
  }
}

void executeWebSocketCommand(String command) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, command);
  
  String action = doc["action"];
  
  if (action == "ignite") {
    int burnerId = doc["burnerId"];
    igniteBurner(burnerId);
  } 
  else if (action == "shutdown") {
    int burnerId = doc["burnerId"];
    shutdownBurner(burnerId);
  }
  else if (action == "master_on") {
    masterSwitch = true;
    igniteSelectedBurners();
  }
  else if (action == "master_off") {
    masterSwitch = false;
    shutdownAllBurners();
  }
  else if (action == "select") {
    JsonArray burnerIds = doc["burnerIds"];
    selectBurners(burnerIds);
  }
  else if (action == "emergency_stop") {
    triggerEmergencyStop();
  }
}

// Burner Control Functions
void igniteBurner(int burnerId) {
  if (burnerId < 1 || burnerId > activeBurners) return;
  if (emergencyStop) return;
  
  Burner* burner = &burners[burnerId - 1];
  
  if (burner->state != BURNER_OFF) {
    Serial.printf("Burner %d already active\n", burnerId);
    return;
  }
  
  Serial.printf("Igniting burner %d...\n", burnerId);
  
  // Start pilot ignition sequence
  burner->state = PILOT_IGNITING;
  burner->ignitionStartTime = millis();
  burner->retryCount++;
  
  // Open pilot valve and activate igniter
  digitalWrite(burner->pilotValvePin, HIGH);
  digitalWrite(burner->igniterPin, HIGH);
  
  logBurnerEvent(burnerId, "Pilot ignition started");
}

void shutdownBurner(int burnerId) {
  if (burnerId < 1 || burnerId > activeBurners) return;
  
  Burner* burner = &burners[burnerId - 1];
  
  Serial.printf("Shutting down burner %d\n", burnerId);
  
  // Turn off all components
  digitalWrite(burner->pilotValvePin, LOW);
  digitalWrite(burner->mainValvePin, LOW);
  digitalWrite(burner->igniterPin, LOW);
  
  burner->state = BURNER_OFF;
  burner->ignitionStartTime = 0;
  burner->retryCount = 0;
  burner->faultCondition = false;
  
  logBurnerEvent(burnerId, "Burner shutdown");
}

void updateBurnerSystem() {
  for (int i = 0; i < activeBurners; i++) {
    Burner* burner = &burners[i];
    
    // Read flame sensor
    int flameReading = analogRead(burner->flameSensorPin);
    bool flameDetected = flameReading > FLAME_THRESHOLD;
    
    // Update temperature (simulated - replace with real sensor)
    if (burner->state >= PILOT_ON) {
      burner->temperature = 150 + (flameReading / 10);
    } else {
      burner->temperature = max(20, burner->temperature - 1);
    }
    
    // State machine logic
    switch (burner->state) {
      case PILOT_IGNITING:
        if (flameDetected) {
          // Pilot lit successfully
          digitalWrite(burner->igniterPin, LOW); // Turn off igniter
          burner->state = PILOT_ON;
          logBurnerEvent(i + 1, "Pilot ignited successfully");
          
          // Start main burner ignition
          delay(1000); // Brief delay
          burner->state = MAIN_IGNITING;
          burner->ignitionStartTime = millis();
          digitalWrite(burner->mainValvePin, HIGH);
          digitalWrite(burner->igniterPin, HIGH);
        }
        else if (millis() - burner->ignitionStartTime > IGNITION_TIMEOUT) {
          // Pilot ignition timeout
          handleIgnitionFailure(burner, i + 1, "Pilot ignition timeout");
        }
        break;
        
      case MAIN_IGNITING:
        if (flameDetected) {
          // Main burner lit successfully
          digitalWrite(burner->igniterPin, LOW);
          burner->state = MAIN_ON;
          burner->retryCount = 0;
          logBurnerEvent(i + 1, "Main burner ignited - READY");
        }
        else if (millis() - burner->ignitionStartTime > IGNITION_TIMEOUT) {
          // Main ignition timeout
          handleIgnitionFailure(burner, i + 1, "Main ignition timeout");
        }
        break;
        
      case PILOT_ON:
      case MAIN_ON:
        if (!flameDetected) {
          // Flame loss detected
          burner->faultCondition = true;
          burner->lastError = "Flame loss detected";
          shutdownBurner(i + 1);
          logBurnerEvent(i + 1, "FAULT: Flame loss - Emergency shutdown");
        }
        break;
        
      default:
        break;
    }
  }
}

void handleIgnitionFailure(Burner* burner, int burnerId, String error) {
  digitalWrite(burner->igniterPin, LOW);
  
  if (burner->retryCount >= MAX_RETRY_ATTEMPTS) {
    // Max retries exceeded - enter fault state
    burner->state = FAULT;
    burner->faultCondition = true;
    burner->lastError = error + " - Max retries exceeded";
    shutdownBurner(burnerId);
    logBurnerEvent(burnerId, "FAULT: " + error);
  } else {
    // Retry ignition
    delay(2000); // Wait before retry
    burner->state = BURNER_OFF;
    logBurnerEvent(burnerId, "Retry " + String(burner->retryCount) + ": " + error);
    igniteBurner(burnerId);
  }
}

void igniteSelectedBurners() {
  Serial.println("Master switch ON - igniting selected burners");
  
  for (int i = 0; i < activeBurners; i++) {
    if (burners[i].selected && burners[i].state == BURNER_OFF) {
      igniteBurner(i + 1);
      delay(500); // Stagger ignitions
    }
  }
}

void shutdownAllBurners() {
  Serial.println("Master switch OFF - shutting down all burners");
  
  for (int i = 0; i < activeBurners; i++) {
    shutdownBurner(i + 1);
  }
}

void selectBurners(JsonArray burnerIds) {
  // Clear all selections
  for (int i = 0; i < activeBurners; i++) {
    burners[i].selected = false;
  }
  
  // Set new selections
  for (int id : burnerIds) {
    if (id >= 1 && id <= activeBurners) {
      burners[id - 1].selected = true;
    }
  }
  
  Serial.print("Selected burners: ");
  for (int i = 0; i < activeBurners; i++) {
    if (burners[i].selected) {
      Serial.print(i + 1);
      Serial.print(" ");
    }
  }
  Serial.println();
}

void handleEmergencyStop() {
  bool emergencyPressed = !digitalRead(EMERGENCY_STOP_PIN);
  
  if (emergencyPressed && !emergencyStop) {
    triggerEmergencyStop();
  }
  
  if (emergencyStop) {
    // Flash status LED during emergency
    digitalWrite(STATUS_LED_PIN, (millis() / 200) % 2);
  }
}

void triggerEmergencyStop() {
  emergencyStop = true;
  masterSwitch = false;
  
  Serial.println("EMERGENCY STOP ACTIVATED");
  
  // Immediate shutdown of all burners
  for (int i = 0; i < MAX_BURNERS; i++) {
    digitalWrite(PILOT_VALVE_PIN + (i * 3), LOW);
    digitalWrite(MAIN_VALVE_PIN + (i * 3), LOW);
    digitalWrite(IGNITER_PIN + (i * 3), LOW);
    burners[i].state = EMERGENCY_STOP;
  }
}

void broadcastStatus() {
  static unsigned long lastBroadcast = 0;
  
  if (millis() - lastBroadcast > 500) { // 2Hz broadcast rate
    DynamicJsonDocument doc(2048);
    
    doc["systemId"] = systemId;
    doc["masterSwitch"] = masterSwitch;
    doc["emergencyStop"] = emergencyStop;
    doc["activeBurners"] = activeBurners;
    doc["timestamp"] = millis();
    
    JsonArray burnersArray = doc.createNestedArray("burners");
    
    for (int i = 0; i < activeBurners; i++) {
      JsonObject burnerObj = burnersArray.createNestedObject();
      burnerObj["id"] = burners[i].id;
      burnerObj["state"] = burners[i].state;
      burnerObj["selected"] = burners[i].selected;
      burnerObj["temperature"] = burners[i].temperature;
      burnerObj["fault"] = burners[i].faultCondition;
      burnerObj["error"] = burners[i].lastError;
    }
    
    String statusJson;
    serializeJson(doc, statusJson);
    
    webSocket.broadcastTXT(statusJson);
    lastBroadcast = millis();
  }
}

void sendSystemStatusToClient(uint8_t clientNum) {
  DynamicJsonDocument doc(2048);
  
  doc["systemId"] = systemId;
  doc["masterSwitch"] = masterSwitch;
  doc["emergencyStop"] = emergencyStop;
  doc["activeBurners"] = activeBurners;
  doc["welcome"] = true;
  
  JsonArray burnersArray = doc.createNestedArray("burners");
  
  for (int i = 0; i < activeBurners; i++) {
    JsonObject burnerObj = burnersArray.createNestedObject();
    burnerObj["id"] = burners[i].id;
    burnerObj["state"] = burners[i].state;
    burnerObj["selected"] = burners[i].selected;
    burnerObj["temperature"] = burners[i].temperature;
    burnerObj["fault"] = burners[i].faultCondition;
    burnerObj["error"] = burners[i].lastError;
  }
  
  String statusJson;
  serializeJson(doc, statusJson);
  
  webSocket.sendTXT(clientNum, statusJson);
}

void logBurnerEvent(int burnerId, String event) {
  String timestamp = String(millis() / 1000);
  Serial.println("[" + timestamp + "] Burner " + String(burnerId) + ": " + event);
}

// HTTP API Handlers
void handleGetBurners(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(2048);
  
  JsonArray burnersArray = doc.createNestedArray("burners");
  
  for (int i = 0; i < activeBurners; i++) {
    JsonObject burnerObj = burnersArray.createNestedObject();
    burnerObj["id"] = burners[i].id;
    burnerObj["state"] = burners[i].state;
    burnerObj["selected"] = burners[i].selected;
    burnerObj["temperature"] = burners[i].temperature;
    burnerObj["fault"] = burners[i].faultCondition;
    burnerObj["error"] = burners[i].lastError;
  }
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
}

void handleIgniteBurner(AsyncWebServerRequest *request) {
  int burnerId = 1;
  if (request->hasParam("id")) {
    burnerId = request->getParam("id")->value().toInt();
  }
  
  igniteBurner(burnerId);
  
  request->send(200, "application/json", "{\"action\":\"ignite\",\"burnerId\":" + String(burnerId) + "}");
}

void handleShutdownBurner(AsyncWebServerRequest *request) {
  int burnerId = 1;
  if (request->hasParam("id")) {
    burnerId = request->getParam("id")->value().toInt();
  }
  
  shutdownBurner(burnerId);
  
  request->send(200, "application/json", "{\"action\":\"shutdown\",\"burnerId\":" + String(burnerId) + "}");
}

void handleMasterSwitch(AsyncWebServerRequest *request) {
  bool state = false;
  if (request->hasParam("state")) {
    state = request->getParam("state")->value() == "true";
  }
  
  masterSwitch = state;
  
  if (state) {
    igniteSelectedBurners();
  } else {
    shutdownAllBurners();
  }
  
  request->send(200, "application/json", "{\"masterSwitch\":" + String(state ? "true" : "false") + "}");
}

void handleBurnerSelection(AsyncWebServerRequest *request) {
  // This would parse burner selection from POST body
  request->send(200, "application/json", "{\"action\":\"selection_updated\"}");
}

void handleSystemStatus(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(1024);
  
  doc["systemId"] = systemId;
  doc["uptime"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["connectedClients"] = WiFi.softAPgetStationNum();
  doc["activeBurners"] = activeBurners;
  doc["masterSwitch"] = masterSwitch;
  doc["emergencyStop"] = emergencyStop;
  
  String response;
  serializeJson(doc, response);
  
  request->send(200, "application/json", response);
}

void handleEmergencyStop(AsyncWebServerRequest *request) {
  triggerEmergencyStop();
  
  request->send(200, "application/json", "{\"emergencyStop\":true}");
}

// Serial command handling function
void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    Serial.printf("\n🔧 Command received: '%s'\n", command.c_str());
    
    if (command == "status") {
      printSystemStatus();
    }
    else if (command == "test") {
      testAllRelays();
    }
    else if (command.startsWith("ignite ")) {
      int burnerNum = command.substring(7).toInt();
      if (burnerNum >= 1 && burnerNum <= activeBurners) {
        igniteBurnerCommand(burnerNum - 1);
      } else {
        Serial.printf("❌ Invalid burner number. Use 1-%d\n", activeBurners);
      }
    }
    else if (command.startsWith("stop ")) {
      int burnerNum = command.substring(5).toInt();
      if (burnerNum >= 1 && burnerNum <= activeBurners) {
        stopBurnerCommand(burnerNum - 1);
      } else {
        Serial.printf("❌ Invalid burner number. Use 1-%d\n", activeBurners);
      }
    }
    else if (command == "emergency") {
      Serial.println("🚨 EMERGENCY STOP ACTIVATED!");
      triggerEmergencyStop();
    }
    else if (command == "reset") {
      Serial.println("🔄 Resetting system...");
      ESP.restart();
    }
    else if (command == "help") {
      printHelp();
    }
    else {
      Serial.printf("❓ Unknown command: '%s'. Type 'help' for commands.\n", command.c_str());
    }
    Serial.println();
  }
}

void printSystemStatus() {
  Serial.println("\n📊 SYSTEM STATUS");
  Serial.println("==================");
  Serial.printf("System ID: %s\n", systemId.c_str());
  Serial.printf("Uptime: %lu ms\n", millis());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("WiFi Clients: %d\n", WiFi.softAPgetStationNum());
  Serial.printf("Emergency Stop: %s\n", emergencyStop ? "ACTIVE" : "NORMAL");
  Serial.printf("Master Switch: %s\n", masterSwitch ? "ON" : "OFF");
  Serial.println("------------------");
  
  for (int i = 0; i < activeBurners; i++) {
    String stateName = "";
    switch(burners[i].state) {
      case BURNER_OFF: stateName = "OFF"; break;
      case PILOT_IGNITING: stateName = "PILOT IGNITING"; break;  
      case PILOT_ON: stateName = "PILOT ON"; break;
      case MAIN_IGNITING: stateName = "MAIN IGNITING"; break;
      case MAIN_ON: stateName = "MAIN ON"; break;
      case BURNER_FAULT: stateName = "FAULT"; break;
      case EMERGENCY_STOP: stateName = "E-STOP"; break;
    }
    
    Serial.printf("Burner %d: %s", i+1, stateName.c_str());
    if (burners[i].temperature > 0) {
      Serial.printf(" | %d°F", burners[i].temperature);
    }
    if (burners[i].fault) {
      Serial.printf(" | FAULT: %s", burners[i].errorMessage.c_str());
    }
    Serial.println();
  }
  Serial.println("==================");
}

void testAllRelays() {
  Serial.println("\n🧪 TESTING ALL RELAYS (LED MODE)");
  Serial.println("==================================");
  
  for (int i = 0; i < activeBurners; i++) {
    Serial.printf("Testing Burner %d relay...\n", i+1);
    
    // Test relay ON (LED should light up)
    digitalWrite(relayPins[i], LOW); // Active LOW
    Serial.printf("  Burner %d relay ON\n", i+1);
    delay(1000);
    
    // Test relay OFF  
    digitalWrite(relayPins[i], HIGH);
    Serial.printf("  Burner %d relay OFF\n", i+1);
    delay(500);
  }
  
  // Test status LED
  Serial.println("Testing status LED...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(200);
    digitalWrite(STATUS_LED_PIN, LOW);  
    delay(200);
  }
  
  Serial.println("✅ Relay test complete");
}

void igniteBurnerCommand(int burnerIndex) {
  if (emergencyStop) {
    Serial.println("❌ Cannot ignite - Emergency stop active");
    return;
  }
  
  if (burners[burnerIndex].state != BURNER_OFF) {
    Serial.printf("❌ Burner %d is not in OFF state\n", burnerIndex + 1);
    return;
  }
  
  Serial.printf("🔥 Igniting Burner %d...\n", burnerIndex + 1);
  burners[burnerIndex].state = PILOT_IGNITING;
  burners[burnerIndex].stateStartTime = millis();
  burners[burnerIndex].selected = true;
}

void stopBurnerCommand(int burnerIndex) {
  Serial.printf("🛑 Stopping Burner %d...\n", burnerIndex + 1);
  burners[burnerIndex].state = BURNER_OFF;
  burners[burnerIndex].stateStartTime = millis();
  burners[burnerIndex].selected = false;
  burners[burnerIndex].fault = false;
  burners[burnerIndex].errorMessage = "";
  
  // Turn off all outputs for this burner
  digitalWrite(relayPins[burnerIndex], HIGH); // Turn off relay
}

void printHelp() {
  Serial.println("\n📝 AVAILABLE SERIAL COMMANDS");
  Serial.println("===============================");
  Serial.println("status       - Show system status");
  Serial.println("test         - Test all relays (LED mode)");  
  Serial.println("ignite X     - Ignite burner X (1-8)");
  Serial.println("stop X       - Stop burner X (1-8)");
  Serial.println("emergency    - Emergency stop all burners");
  Serial.println("reset        - Reset ESP32-C6 system");
  Serial.println("help         - Show this help message");
  Serial.println("===============================");
  Serial.printf("💡 You have %d active burners\n", activeBurners);
  Serial.println("💡 Use LED mode for safe testing before real hardware");
}
