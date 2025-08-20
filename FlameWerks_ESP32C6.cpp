#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLECharacteristic.h>
#include <array>
#include <algorithm>
#include <esp_system.h>
#include <esp_task_wdt.h>

// --- Pin Definitions (SparkFun ESP32-C6 Thing Plus) ---
#define PILOT_RELAY_PIN  2    // GPIO2 -> Pilot Valve Relay
#define MAIN_RELAY_PIN   4    // GPIO4 -> Main Valve Relay
#define GLOW_PLUG_PIN    5    // GPIO5 -> 12V Glow Plug Control
#define FLAME_SENSE_PIN  1    // A1: ADC1_CH1 -> Flame Rod Input (confirm Thing Plus A1 pin mapping)
#define ESTOP_INPUT_PIN  9    // GPIO9 -> Emergency Stop Input (NC to GND)
#define STATUS_LED_PIN   8    // GPIO8 -> Status LED (Blue/Green indication)

// --- BLE UUIDs (from dev plan) ---
#define SERVICE_UUID "b43a0001-5f7a-4a9b-9c8a-0000f10a0001"
#define COMMAND_CHAR_UUID "b43a1001-5f7a-4a9b-9c8a-0000f10a1001"
#define STATUS_CHAR_UUID "b43a1002-5f7a-4a9b-9c8a-0000f10a1002"
#define TELEMETRY_CHAR_UUID "b43a1003-5f7a-4a9b-9c8a-0000f10a1003"
#define CONFIG_CHAR_UUID "b43a1004-5f7a-4a9b-9c8a-0000f10a1004"

// --- Command Opcodes ---
enum class CommandOpcode {
  ARM = 0x01,
  IGNITE = 0x02,
  VALVE_MAIN_ON = 0x03,
  VALVE_MAIN_OFF = 0x04,
  STOP = 0x05,
  RESET = 0x06,
  SET_CONFIG = 0x10,
  GET_CONFIG = 0x11,
  SELECT_BURNER = 0x20
};

// --- Connection and LED Status ---
enum class ConnectionState {
  WAITING_TO_CONNECT,  // Blue flashing LED
  CONNECTING,         // Solid blue LED
  CONNECTED          // Solid green LED
};

ConnectionState connectionState = ConnectionState::WAITING_TO_CONNECT;
unsigned long lastLedUpdate = 0;
bool ledState = false;

// --- LED Control Functions ---
void setStatusLED(int red, int green, int blue) {
  // For RGB LED control - adjust based on your actual LED setup
  // This is a placeholder - you may need to use analogWrite or specific LED library
  if (red > 0 && green == 0 && blue > 0) {
    // Purple/Magenta for mixed
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else if (green > 0) {
    // Green
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else if (blue > 0) {
    // Blue
    digitalWrite(STATUS_LED_PIN, HIGH);
  } else {
    digitalWrite(STATUS_LED_PIN, LOW);
  }
}

void updateStatusLED() {
  unsigned long currentTime = millis();
  
  switch (connectionState) {
    case ConnectionState::WAITING_TO_CONNECT:
      // Flash blue every 500ms
      if (currentTime - lastLedUpdate > 500) {
        ledState = !ledState;
        if (ledState) {
          setStatusLED(0, 0, 255); // Blue
        } else {
          setStatusLED(0, 0, 0);   // Off
        }
        lastLedUpdate = currentTime;
      }
      break;
      
    case ConnectionState::CONNECTING:
      // Solid blue
      setStatusLED(0, 0, 255); // Blue
      break;
      
    case ConnectionState::CONNECTED:
      // Solid green
      setStatusLED(0, 255, 0); // Green
      break;
  }
}

// --- Burner State Machine ---
enum class BurnerState {
  OFF,                // All systems off, safe state
  PILOT_IGNITING,     // Opening pilot valve + glow plug
  PILOT_ESTABLISHED,  // Pilot flame confirmed
  VALVE_MAIN_ON,      // Main valve open, normal operation
  FLAMEOUT,          // Flame lost, cooldown period
  FAULT              // System fault, manual reset required
};

// --- Burner Control Structure ---
struct BurnerController {
  uint8_t id;
  BurnerState state;
  unsigned long stateStartTime;
  uint8_t retryAttempt;
  bool flameDetected;
  bool pilotValveOpen;
  bool mainValveOpen;
  bool glowPlugActive;
  int lastFlameReading;
  bool selected;  // For multi-burner control
};

// Maximum number of burners (40 as per requirements)
const uint8_t MAX_BURNERS = 40;
std::array<BurnerController, MAX_BURNERS> burners;
uint8_t activeBurnerCount = 0; // Start with 0, will be set when connected and configured
uint8_t configuredBurnerCount = 1; // Default to 1 burner until app tells us otherwise
bool burnersInitialized = false;
bool systemArmed = false;           // Reflects ARM/STOP for UI state (app shows ARMED when OFF + armed)
uint8_t lastFaultCode = 0;          // Simple fault code for UI (0 = none)
// Number of FlameWerks devices connected to the same phone (sent by the app)
uint8_t groupConnectedCount = 0;
// This device's burner index (assign via app or manually via SET_CONFIG key 0x03)
uint8_t myBurnerIndex = 1;

// --- Safety Parameters ---
const unsigned long PILOT_IGNITION_TIME_MS = 5000;   // 5 seconds for pilot ignition
const unsigned long PILOT_STABILIZE_TIME_MS = 2000;  // 2 seconds to stabilize pilot
const unsigned long FLAME_CHECK_INTERVAL_MS = 100;   // Check flame every 100ms
// ADC threshold for flame-present decision (12-bit ADC, 0..4095). Tunable in field.
const int FLAME_ADC_THRESHOLD = 150;                 // ~0.12V with 3.3V ref
const unsigned long FLAME_LOSS_TIMEOUT_MS = 500;     // 500ms to detect flame loss
const unsigned long COOLDOWN_PERIOD_MS = 3000;       // 3 second cooldown between attempts
const uint8_t MAX_IGNITION_ATTEMPTS = 3;            // Maximum retry attempts
const int FLAME_DETECTION_THRESHOLD = 80;           // Flame detection threshold

// BLE Server pointers
NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pCommandCharacteristic = nullptr;
NimBLECharacteristic* pStatusCharacteristic = nullptr;
NimBLECharacteristic* pTelemetryCharacteristic = nullptr;
NimBLECharacteristic* pConfigCharacteristic = nullptr;
bool bleReady = false;
static bool gWdtAdded = false; // track if current task is registered with WDT

// --- Simulated Hardware Control Functions ---
void setPilotValve(uint8_t burnerId, bool state) {
  if (burnerId >= activeBurnerCount) return;
  burners[burnerId].pilotValveOpen = state;
}

void setMainValve(uint8_t burnerId, bool state) {
  if (burnerId >= activeBurnerCount) return;
  burners[burnerId].mainValveOpen = state;
}

void setGlowPlug(uint8_t burnerId, bool state) {
  if (burnerId >= activeBurnerCount) return;
  burners[burnerId].glowPlugActive = state;
}

bool readFlameStatus(uint8_t burnerId) {
  if (burnerId >= activeBurnerCount) return false;

  // Sample ADC with a small moving average to reduce noise from ignition/EMI
  static unsigned long lastSampleMs = 0;
  static int filtered = 0;

  unsigned long now = millis();
  if (now - lastSampleMs >= FLAME_CHECK_INTERVAL_MS) {
    const int samples = 8;
    long sum = 0;
    for (int i = 0; i < samples; ++i) {
      sum += analogRead(FLAME_SENSE_PIN);
      delayMicroseconds(200);
    }
    int avg = (int)(sum / samples);
    // Simple IIR: filtered = 3/4 old + 1/4 new
    filtered = (filtered == 0) ? avg : ((filtered * 3 + avg) / 4);
    burners[burnerId].lastFlameReading = filtered;
    lastSampleMs = now;
  }

  return burners[burnerId].lastFlameReading >= FLAME_ADC_THRESHOLD;
}

void emergencyShutdown(uint8_t burnerId) {
  setGlowPlug(burnerId, false);
  setPilotValve(burnerId, false);
  setMainValve(burnerId, false);
  burners[burnerId].state = BurnerState::OFF;
  burners[burnerId].retryAttempt = 0;
}

void emergencyStopAll() {
  for (uint8_t i = 0; i < activeBurnerCount; i++) {
    emergencyShutdown(i);
  }
}

// --- BLE Callbacks ---
class ServerCallbacks: public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) {
    connectionState = ConnectionState::CONNECTING;
    Serial.println("Client connecting...");
    
    // Initialize burners when client connects
    if (!burnersInitialized) {
      activeBurnerCount = configuredBurnerCount;
      Serial.println("Connecting...");
      Serial.println("Reading burner configuration...");
      Serial.print("Configured burners: ");
      Serial.println(activeBurnerCount);
      Serial.println("Assigning burner number...");
      
      // Initialize burner assignment display
      if (activeBurnerCount > 0) {
        Serial.print("Assigned to Burner #");
        Serial.println(myBurnerIndex);
      }
      
      burnersInitialized = true;
    }
    
    connectionState = ConnectionState::CONNECTED;
    Serial.println("Connected!");
    Serial.println("Flame Status: Monitoring...");
  }
  
  void onDisconnect(NimBLEServer* pServer) {
    connectionState = ConnectionState::WAITING_TO_CONNECT;
    Serial.println("Client disconnected");
    Serial.println("Waiting to connect...");
    Serial.println("Restart advertising");
    {
      NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  if (adv && !adv->isAdvertising()) adv->start();
    }
  }
};

class CommandCallbacks: public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      uint8_t opcode = value[0];
      uint8_t burnerId = 0;
      
      // Extract burner ID if provided (for multi-burner commands)
      if (value.length() > 1) {
        burnerId = value[1];
        if (burnerId >= activeBurnerCount) return;
      }
      
      switch (static_cast<CommandOpcode>(opcode)) {
        case CommandOpcode::ARM:
          systemArmed = true;
          if (burners[burnerId].state == BurnerState::OFF || burners[burnerId].state == BurnerState::FAULT) {
            burners[burnerId].state = BurnerState::OFF;
            burners[burnerId].retryAttempt = 0;
          }
          break;
          
        case CommandOpcode::IGNITE:
          if (burners[burnerId].state == BurnerState::OFF) {
            burners[burnerId].state = BurnerState::PILOT_IGNITING;
            burners[burnerId].stateStartTime = millis();
          }
          break;
          
        case CommandOpcode::VALVE_MAIN_ON:
          if (burners[burnerId].state == BurnerState::PILOT_ESTABLISHED) {
            burners[burnerId].state = BurnerState::VALVE_MAIN_ON;
          }
          break;
          
        case CommandOpcode::VALVE_MAIN_OFF:
          if (burners[burnerId].state == BurnerState::VALVE_MAIN_ON) {
            setMainValve(burnerId, false);
            burners[burnerId].state = BurnerState::PILOT_ESTABLISHED;
          }
          break;
          
        case CommandOpcode::STOP:
          emergencyShutdown(burnerId);
          systemArmed = false;
          break;
          
        case CommandOpcode::RESET:
          if (burners[burnerId].state == BurnerState::FAULT) {
            burners[burnerId].state = BurnerState::OFF;
            burners[burnerId].retryAttempt = 0;
            lastFaultCode = 0;
          }
          break;
          
        case CommandOpcode::SELECT_BURNER:
          if (burnerId < activeBurnerCount) {
            for (auto& burner : burners) {
              burner.selected = false;
            }
            burners[burnerId].selected = true;
          }
          break;
          
        case CommandOpcode::SET_CONFIG:
          if (value.length() >= 3) {
            uint8_t configKey = value[1];
            uint8_t configValue = value[2];
            
            // Handle burner count configuration
            if (configKey == 0x01) { // Burner count config
              if (configValue <= MAX_BURNERS && configValue > 0) {
                configuredBurnerCount = configValue;
                if (burnersInitialized) {
                  activeBurnerCount = configuredBurnerCount;
                  Serial.print("Updated burner count to: ");
                  Serial.println(activeBurnerCount);
                }
              }
            } else if (configKey == 0x02) { // Group connected devices count (from phone)
              groupConnectedCount = configValue;
              Serial.print("[APP] Group connected devices set to: ");
              Serial.println(groupConnectedCount);
            } else if (configKey == 0x03) { // Assign this device's burner index
              if (configValue == 0) configValue = 1; // clamp to 1..MAX_BURNERS
              myBurnerIndex = std::min<uint8_t>(configValue, MAX_BURNERS);
              Serial.print("[APP] Burner index assigned: ");
              Serial.println(myBurnerIndex);
            }
          }
          break;
      }
    }
  }
};

// --- Status Updates ---
void updateStatus() {
  if (!pServer || !burnersInitialized) return;
  if (!pStatusCharacteristic || !pTelemetryCharacteristic) return;

  // Choose the selected burner if any, otherwise burner 0
  uint8_t idx = 0;
  for (uint8_t i = 0; i < activeBurnerCount; i++) {
    if (burners[i].selected) { idx = i; break; }
  }

  // Map BurnerState to app state codes: 0=OFF, 1=ARMED, 2=IGNITING, 3=MAIN ON, 4=FAULT
  auto mapState = [](BurnerState s) -> uint8_t {
    if (s == BurnerState::FAULT) return 4;
    if (s == BurnerState::VALVE_MAIN_ON) return 3;
    if (s == BurnerState::PILOT_IGNITING) return 2;
    // Treat OFF + armed as ARMED (1), otherwise OFF (0) or PILOT_ESTABLISHED (consider ARMED)
    if (s == BurnerState::OFF) return systemArmed ? 1 : 0;
    if (s == BurnerState::PILOT_ESTABLISHED) return systemArmed ? 1 : 0;
    if (s == BurnerState::FLAMEOUT) return systemArmed ? 1 : 0;
    return 0;
  };

  uint8_t status[8];
  status[0] = mapState(burners[idx].state);
  uint32_t uptime = millis();
  status[1] = (uint8_t)(uptime & 0xFF);
  status[2] = (uint8_t)((uptime >> 8) & 0xFF);
  status[3] = (uint8_t)((uptime >> 16) & 0xFF);
  status[4] = (uint8_t)((uptime >> 24) & 0xFF);
  status[5] = burners[idx].retryAttempt;
  status[6] = burners[idx].flameDetected ? 1 : 0;
  status[7] = lastFaultCode;

  pStatusCharacteristic->setValue(status, sizeof(status));
  // Only notify if there is at least one connected client
  if (pServer->getConnectedCount() > 0) {
    pStatusCharacteristic->notify();
  }

  // Telemetry 8 bytes: [flameCurrentQ10 u16][adcRaw u16][batteryMv u16][temperatureC*100 i16]
  uint8_t telemetry[8];
  // Simulate flameCurrent in uA using lastFlameReading (0-200). Clamp and scale to Q10.
  uint16_t flameCurrentUA_Q10 = (uint16_t)(std::min(60, std::max(0, burners[idx].lastFlameReading)) * 1024);
  uint16_t adcRaw = (uint16_t)std::min(1023, burners[idx].lastFlameReading * 10);
  uint16_t batteryMv = 3800; // Simulated 3.8V
  int16_t tempC_times100 = 2500; // 25.00 C

  telemetry[0] = (uint8_t)(flameCurrentUA_Q10 & 0xFF);
  telemetry[1] = (uint8_t)((flameCurrentUA_Q10 >> 8) & 0xFF);
  telemetry[2] = (uint8_t)(adcRaw & 0xFF);
  telemetry[3] = (uint8_t)((adcRaw >> 8) & 0xFF);
  telemetry[4] = (uint8_t)(batteryMv & 0xFF);
  telemetry[5] = (uint8_t)((batteryMv >> 8) & 0xFF);
  telemetry[6] = (uint8_t)(tempC_times100 & 0xFF);
  telemetry[7] = (uint8_t)((tempC_times100 >> 8) & 0xFF);

  pTelemetryCharacteristic->setValue(telemetry, sizeof(telemetry));
  if (pServer->getConnectedCount() > 0) {
    pTelemetryCharacteristic->notify();
  }
}

// --- State Machine Update ---
void updateBurnerStateMachine(uint8_t burnerId) {
  if (burnerId >= activeBurnerCount) return;
  
  unsigned long currentTime = millis();
  BurnerController& burner = burners[burnerId];
  
  // Read flame status
  burner.flameDetected = readFlameStatus(burnerId);
  
  // Check emergency stop input
  if (digitalRead(ESTOP_INPUT_PIN) == HIGH) { // NC contact opened
    emergencyStopAll();
    return;
  }
  
  switch (burner.state) {
    case BurnerState::OFF:
      // Safe state - all systems off
      setGlowPlug(burnerId, false);
      setPilotValve(burnerId, false);
      setMainValve(burnerId, false);
      break;
      
    case BurnerState::PILOT_IGNITING:
      // Try to establish pilot flame
      setPilotValve(burnerId, true);
      setMainValve(burnerId, false);
      setGlowPlug(burnerId, true);
      
      if (burner.flameDetected) {
        burner.state = BurnerState::PILOT_ESTABLISHED;
        burner.stateStartTime = currentTime;
        burner.retryAttempt = 0;
      }
      else if (currentTime - burner.stateStartTime > PILOT_IGNITION_TIME_MS) {
        setGlowPlug(burnerId, false);
        setPilotValve(burnerId, false);
        
        if (burner.retryAttempt < MAX_IGNITION_ATTEMPTS) {
          burner.retryAttempt++;
          burner.state = BurnerState::FLAMEOUT;
          burner.stateStartTime = currentTime;
        } else {
          burner.state = BurnerState::FAULT;
        }
      }
      break;
      
    case BurnerState::PILOT_ESTABLISHED:
      // Pilot flame stable, ready for main valve
      setPilotValve(burnerId, true);
      setMainValve(burnerId, false);
      setGlowPlug(burnerId, false);
      
      if (!burner.flameDetected) {
        emergencyShutdown(burnerId);
        burner.state = BurnerState::FLAMEOUT;
        burner.stateStartTime = currentTime;
      }
      break;
      
    case BurnerState::VALVE_MAIN_ON:
      // Normal operation with main valve
      setPilotValve(burnerId, true);
      setMainValve(burnerId, true);
      setGlowPlug(burnerId, false);
      
      if (!burner.flameDetected) {
        emergencyShutdown(burnerId);
        burner.state = BurnerState::FLAMEOUT;
        burner.stateStartTime = currentTime;
  lastFaultCode = 1; // simple flameout fault
      }
      break;
      
    case BurnerState::FLAMEOUT:
      // Cooldown period before retry
      setGlowPlug(burnerId, false);
      setPilotValve(burnerId, false);
      setMainValve(burnerId, false);
      
      if (currentTime - burner.stateStartTime > COOLDOWN_PERIOD_MS) {
        if (burner.retryAttempt < MAX_IGNITION_ATTEMPTS) {
          burner.state = BurnerState::PILOT_IGNITING;
          burner.stateStartTime = currentTime;
        } else {
          burner.state = BurnerState::FAULT;
        }
      }
      break;
      
    case BurnerState::FAULT:
      // System fault - requires manual reset
      setGlowPlug(burnerId, false);
      setPilotValve(burnerId, false);
      setMainValve(burnerId, false);
      break;
  }
}

// For simulation, give each board a unique name from efuse MAC (stable across boots)
String getDeviceName() {
  uint64_t chipId = ESP.getEfuseMac();
  uint32_t lastThreeBytes = (uint32_t)(chipId & 0xFFFFFF);
  char name[32];
  snprintf(name, sizeof(name), "FlameWerks_%06X", lastThreeBytes);
  return String(name);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== FlameWerks Multi-Burner Simulator ===");
  Serial.println("SparkFun ESP32-C6 Thing Plus - BLE Test Version");
  Serial.println("Waiting to connect...");
  Serial.print("Reset reason: ");
  Serial.println((int)esp_reset_reason());
  Serial.print("Free heap (start): ");
  Serial.println(ESP.getFreeHeap());
  // Setup a lightweight watchdog to auto-recover from rare deadlocks (ESP-IDF v5 API)
  const int WDT_TIMEOUT_SEC = 5; // keep modest
  esp_task_wdt_config_t wdt_cfg = {
    .timeout_ms = WDT_TIMEOUT_SEC * 1000,
    .trigger_panic = true,
  };
  esp_err_t wdtInit = esp_task_wdt_init(&wdt_cfg);
  if (wdtInit == ESP_OK || wdtInit == ESP_ERR_INVALID_STATE) {
    // INVALID_STATE means WDT already initialized by core; just add current task
    if (esp_task_wdt_add(NULL) == ESP_OK) {
      gWdtAdded = true;
    } else {
      Serial.println("WARN: esp_task_wdt_add failed; watchdog will be disabled for loop task");
    }
  } else {
    Serial.print("WARN: esp_task_wdt_init failed (err="); Serial.print((int)wdtInit); Serial.println(")");
  }
  
  // Initialize status LED pin
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
  // Configure analog input for flame sensing
  pinMode(FLAME_SENSE_PIN, INPUT);
#ifdef analogReadResolution
  analogReadResolution(12);
#endif
#ifdef analogSetAttenuation
  // 11dB supports ~3.3V full-scale on ESP32-family
  analogSetAttenuation(ADC_11db);
#endif
  // Ensure ESTOP input is pulled up to avoid floating
  pinMode(ESTOP_INPUT_PIN, INPUT_PULLUP);
  
  // Initialize connection state
  connectionState = ConnectionState::WAITING_TO_CONNECT;
  
  // Start with 1 burner by default, will be configured when connected
  configuredBurnerCount = 1;
  activeBurnerCount = 0; // Not initialized until connected
  burnersInitialized = false;
  
  // Initialize burner controls (no hardware pins needed for simulation)
  for (uint8_t i = 0; i < MAX_BURNERS; i++) {
    
    burners[i] = {
      i,                  // id
      BurnerState::OFF,  // state
      0,                 // stateStartTime
      0,                 // retryAttempt
      false,            // flameDetected
      false,            // pilotValveOpen
      false,            // mainValveOpen
      false,            // glowPlugActive
      0,                // lastFlameReading
      false             // selected
    };
  }
  
  // Initialize BLE then compute a unique name
  Serial.println("Init pins done");
  delay(50);
  Serial.println("Initializing BLE controller...");
  NimBLEDevice::init("FlameWerks"); // temporary name
  Serial.println("NimBLE init OK");
  // Reduce MTU a bit to limit buffer usage when multiple boards are active
  NimBLEDevice::setMTU(69);
  Serial.print("Free heap (post NimBLE init): ");
  Serial.println(ESP.getFreeHeap());
  String deviceName = getDeviceName();
  NimBLEDevice::setDeviceName(deviceName.c_str());
  Serial.print("Device Name: "); Serial.println(deviceName);
  
  // Create the BLE Server
  Serial.println("Creating BLE Server...");
  pServer = NimBLEDevice::createServer();
  if (!pServer) {
    Serial.println("ERROR: createServer() returned null");
  } else {
    pServer->setCallbacks(new ServerCallbacks());
  }
  
  // Create the BLE Service
  if (!pServer) {
    Serial.println("FATAL: BLE server not available; skipping service creation");
  }
  NimBLEService* pService = pServer ? pServer->createService(SERVICE_UUID) : nullptr;
  if (!pService) {
    Serial.println("ERROR: createService() returned null");
  }
  
  // Create BLE Characteristics
  if (pService) {
  pCommandCharacteristic = pService->createCharacteristic(
    COMMAND_CHAR_UUID,
    NIMBLE_PROPERTY::WRITE
  );
  if (pCommandCharacteristic) pCommandCharacteristic->setCallbacks(new CommandCallbacks());
  
  pStatusCharacteristic = pService->createCharacteristic(
    STATUS_CHAR_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  
  pTelemetryCharacteristic = pService->createCharacteristic(
    TELEMETRY_CHAR_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  
  pConfigCharacteristic = pService->createCharacteristic(
    CONFIG_CHAR_UUID,
    NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
  );
  }
  
  // Start the service
  if (pService) {
    pService->start();
  }
  
  // Configure advertising so scanners filtering by our service see us
  Serial.println("Starting service and advertising setup...");
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  bool advStarted = false;
  if (pAdvertising) {
  // Build advertisement within 31 bytes budget to avoid crashes on some stacks
  //  - Put the 128-bit service UUID in the primary advertisement
  //  - Move the (long) device name to the scan response packet
  NimBLEAdvertisementData advData;
  advData.addServiceUUID(SERVICE_UUID);
  pAdvertising->setAdvertisementData(advData);

  NimBLEAdvertisementData scanData;
  scanData.setName(deviceName.c_str());
  pAdvertising->setScanResponseData(scanData);
    // Optional: set advertising interval (~100-150ms)
    pAdvertising->setMinInterval(160);
    pAdvertising->setMaxInterval(240);
    advStarted = pAdvertising->start();
    bleReady = true;
  } else {
    Serial.println("ERROR: getAdvertising() returned null");
  }
  Serial.print("BLE Address: ");
  Serial.println(NimBLEDevice::getAddress().toString().c_str());
  Serial.print("Advertising start: ");
  Serial.println(advStarted ? "OK" : "FAILED");
  Serial.print("Free heap (post adv): ");
  Serial.println(ESP.getFreeHeap());
  
  Serial.println("BLE Server started");
  Serial.println("Device Name: " + deviceName);
  Serial.println("Status: Waiting to connect...");
  Serial.println("LED: Blue flashing (waiting for connection)");
  Serial.println("===============================");
}

void loop() {
  // Feed watchdog early to avoid false triggers (only if added)
  if (gWdtAdded) esp_task_wdt_reset();
  // Reconcile connection state with actual connected count in case callbacks are missed
  int connected = pServer ? pServer->getConnectedCount() : 0;
  static int lastConnected = -1;
  if (connected != lastConnected) {
    Serial.print("[BLE] Connected clients: ");
    Serial.println(connected);
    lastConnected = connected;
  }
  if (connected > 0 && connectionState != ConnectionState::CONNECTED) {
    connectionState = ConnectionState::CONNECTED;
    Serial.println("[BLE] State -> CONNECTED (reconciled)");
    if (!burnersInitialized) {
      activeBurnerCount = configuredBurnerCount;
      burnersInitialized = true;
      Serial.print("Active Burners: "); Serial.println(activeBurnerCount);
    }
    // Fallback group count when app isn't running: show 1 if we have a client
    if (groupConnectedCount == 0) {
      groupConnectedCount = 1;
    }
  } else if (connected == 0 && connectionState != ConnectionState::WAITING_TO_CONNECT) {
    connectionState = ConnectionState::WAITING_TO_CONNECT;
    Serial.println("[BLE] State -> WAITING_TO_CONNECT (reconciled)");
    {
      NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  if (adv && !adv->isAdvertising()) adv->start();
    }
  }

  // Update status LED based on connection state
  updateStatusLED();
  
  // Update all active burners (only if initialized)
  if (burnersInitialized) {
    for (uint8_t i = 0; i < activeBurnerCount; i++) {
      updateBurnerStateMachine(i);
    }
  }
  
  // Update BLE status and telemetry (throttled to every 100ms)
  static unsigned long lastUpdate = 0;
  static unsigned long lastSerialStatus = 0;
  
  if (millis() - lastUpdate > 100) {
    updateStatus();
    lastUpdate = millis();
  }

  // Print status to serial monitor every 15 seconds
  if (millis() - lastSerialStatus > 15000) {
    Serial.println("\n=== FlameWerks Status Update ===");
    
    // Show connection status
    String connStatus;
    switch (connectionState) {
      case ConnectionState::WAITING_TO_CONNECT:
        connStatus = "Waiting to connect...";
        break;
      case ConnectionState::CONNECTING:
        connStatus = "Connecting...";
        break;
      case ConnectionState::CONNECTED:
        connStatus = "Connected";
        break;
    }
    Serial.println("Connection Status: " + connStatus);
  Serial.print("Connected (this device): ");
  Serial.println(String(pServer ? pServer->getConnectedCount() : 0));
  Serial.print("Devices connected to phone: ");
  Serial.println(String(groupConnectedCount));
    
    if (burnersInitialized) {
      Serial.println("Active Burners: " + String(activeBurnerCount));
      Serial.println("Flame Status: Monitoring");
      
      // Print status for each active burner
      for (uint8_t i = 0; i < activeBurnerCount; i++) {
        Serial.println("\nBurner " + String(i + 1) + " Status:");
        
        String stateStr;
        switch (burners[i].state) {
          case BurnerState::OFF: stateStr = "OFF"; break;
          case BurnerState::PILOT_IGNITING: stateStr = "PILOT_IGNITING"; break;
          case BurnerState::PILOT_ESTABLISHED: stateStr = "PILOT_ESTABLISHED"; break;
          case BurnerState::VALVE_MAIN_ON: stateStr = "MAIN_ON"; break;
          case BurnerState::FLAMEOUT: stateStr = "FLAMEOUT"; break;
          case BurnerState::FAULT: stateStr = "FAULT"; break;
          default: stateStr = "UNKNOWN"; break;
        }
        
        Serial.println("  State: " + stateStr);
        Serial.println("  Flame Detected: " + String(burners[i].flameDetected ? "YES" : "NO"));
        Serial.println("  Flame Reading: " + String(burners[i].lastFlameReading));
        Serial.println("  Pilot Valve: " + String(burners[i].pilotValveOpen ? "OPEN" : "CLOSED"));
        Serial.println("  Main Valve: " + String(burners[i].mainValveOpen ? "OPEN" : "CLOSED"));
      }
    } else {
      Serial.println("Burners: Not initialized (waiting for connection)");
    }
    Serial.println("===============================");
    lastSerialStatus = millis();
  }
  
  // Emergency stop check
  if (digitalRead(ESTOP_INPUT_PIN) == HIGH) {
    emergencyStopAll();
  }
  
  // Small delay to prevent excessive CPU usage
  delay(10);
}
