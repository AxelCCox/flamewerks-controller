# FlameWerks Development - Quick Implementation Guide

## 🚀 GET STARTED IN 30 MINUTES - ESP32-C6 MASTER-SLAVE ARCHITECTURE

### **Step 1: Hardware Setup - Master Board (15 minutes)**
```bash
# Order Master Controller Components:
- SparkFun ESP32-C6 Thing Plus (~$25)
- 128x64 OLED Display (~$8)
- RGB Status LED (~$3)
- Buzzer for alerts (~$5)
- MicroSD card module (~$8)
- 12V 3A power supply (~$18)
- Buck converter 12V→3.3V (~$12)

# Master Board Wiring:
ESP32-C6 GPIO8  → OLED SDA
ESP32-C6 GPIO9  → OLED SCL
ESP32-C6 GPIO10 → Status LED (WS2812)
ESP32-C6 GPIO11 → Buzzer
ESP32-C6 GPIO12 → SD Card CS
ESP32-C6 GPIO13 → Emergency Stop Input
12V → Buck converter → ESP32-C6 (3.3V)
```

### **Step 2: Hardware Setup - Slave Burner Boards (10 minutes each)**
```bash
# Order Per-Burner Components:
- SparkFun ESP32-C6 Thing Plus (~$25)
- 1MΩ resistors for flame sensing (~$2)
- 220Ω resistors for relay control (~$2)
- 5V relay modules, 2-pack (~$12)
- PC817 optocouplers (~$8)
- Status LED strip (~$5)

# Slave Board Wiring (per burner):
ESP32-C6 GPIO4  → Relay 1 (Pilot valve)
ESP32-C6 GPIO5  → Relay 2 (Main valve)
ESP32-C6 GPIO6  → PC817 → Igniter circuit
ESP32-C6 GPIO7  → 1MΩ → Flame rod sensing
ESP32-C6 GPIO8  → Status LED strip
ESP32-C6 GPIO9  → Temperature sensor (optional)
12V → Buck converter → ESP32-C6 (3.3V)
```

### **Step 3: ESP32-C6 Firmware Architecture (20 minutes)**
```bash
# Install Arduino IDE with ESP32-C6 support:
# File → Preferences → Additional Board Manager URLs:
# https://espressif.github.io/arduino-esp32/package_esp32_index.json

# Install required libraries:
# - ESP32 BLE Arduino
# - ArduinoJson
# - WiFi
# - ESPAsyncWebServer

# Upload firmware:
# Master: ESP32_C6_Master_Controller.ino
# Slave: ESP32_C6_Burner_Slave.ino
```

### **Step 4: Mobile App Setup (15 minutes)**
```bash
# Install Flutter SDK
# Create new Flutter project with BLE capabilities

# Copy provided files:
pubspec.yaml           → Root directory
lib/main.dart         → Main app entry
lib/models/           → Device and burner models
lib/services/         → BLE service handlers
lib/screens/          → UI screens
lib/widgets/          → Custom widgets

# Run commands:
flutter pub get
flutter run

# App discovers "FlameWerks-Master" automatically
# Master reports all connected slave burners
```

---

## 🏗️ MASTER-SLAVE ARCHITECTURE

### **System Overview:**
```
📱 Mobile App (Flutter)
    ↓ Bluetooth BLE
🎛️ Master ESP32-C6 Controller
    ↓ ESP-NOW Mesh Network
🔥 Slave Burner 1 (ESP32-C6)
🔥 Slave Burner 2 (ESP32-C6)
🔥 Slave Burner 3 (ESP32-C6)
    ... up to 40 burners
```

### **Communication Protocol:**
```cpp
// Mobile ↔ Master (Bluetooth BLE)
Commands: START_BURNER, STOP_BURNER, EMERGENCY_STOP, GET_STATUS
Response: ACK, NACK, STATUS_UPDATE, ALARM

// Master ↔ Slaves (ESP-NOW)
Commands: IGNITE, SHUTDOWN, PING, GET_FLAME_STATUS
Response: FLAME_ON, FLAME_OFF, ALIVE, ERROR, SENSOR_DATA
```

### **Auto-Discovery System:**
```cpp
// New burner installation process:
1. Power on new ESP32-C6 slave board
2. Slave broadcasts "FLAMEWERKS_NEW_DEVICE" beacon
3. Master detects new device and assigns ID
4. Slave stores ID in EEPROM for future use
5. Master updates mobile app with new burner
6. User assigns burner name/location in app
```

---

## 📡 ESP32-C6 ADVANTAGES FOR THIS PROJECT

### **Hardware Comparison:**
```
ESP32 (Previous):         ESP32-C6 (Current):
❌ WiFi 4 only            ✅ WiFi 6 + Bluetooth 5
❌ 240MHz single/dual     ✅ 160MHz RISC-V core
❌ Basic mesh support     ✅ Native mesh networking
❌ 34 GPIO pins           ✅ 30 GPIO pins (sufficient)
❌ No Zigbee/Thread       ✅ Zigbee 3.0 + Thread ready
❌ Higher power draw      ✅ Ultra-low power modes
```

### **Mesh Network Benefits:**
```cpp
// ESP-NOW advantages for burner network:
✅ 250-byte packets (perfect for commands)
✅ 1km+ range in open space
✅ No WiFi router dependency
✅ Self-healing mesh topology
✅ <5ms latency for emergency stops
✅ Encrypted peer-to-peer communication
```

---

## 🔧 SCALABLE BURNER MANAGEMENT

### **Dynamic Device Registration:**
```cpp
// Master Controller Logic:
struct BurnerDevice {
    uint8_t mac_address[6];
    uint8_t assigned_id;
    char friendly_name[32];
    uint32_t last_seen;
    BurnerStatus status;
    FlameState flame_state;
};

// Auto-assign IDs 1-40 as devices come online
BurnerDevice burner_registry[40];
```

### **Mobile App Device Discovery:**
```dart
// Flutter BLE Service
class FlameWerksController {
  List<BurnerDevice> connectedBurners = [];
  
  // Master reports all slaves via single BLE connection
  void onMasterStatusUpdate(Map<String, dynamic> data) {
    connectedBurners = parseBurnerList(data['burners']);
    updateUI();
  }
  
  // Add new burner through master
  Future<bool> addNewBurner(String friendlyName) async {
    return await sendToMaster('ADD_BURNER', {'name': friendlyName});
  }
}
```

### **Progressive Installation Support:**
```bash
# Installation Timeline (Restaurant/Brewery):
Week 1: Install master + 2 burners (pilot system)
Week 3: Add 4 more burners (expansion)
Month 2: Add 10 more burners (full kitchen)
Month 6: Add 8 more burners (outdoor area)
Year 2: Add remaining burners as needed

# Each addition is plug-and-play:
1. Mount new ESP32-C6 slave board
2. Power on (auto-discovers master)
3. Master notifies mobile app
4. User assigns name in app
5. Ready to use immediately
```

---

## 🛡️ ENHANCED SAFETY FEATURES

### **Multi-Level Safety System:**
```cpp
// Slave Board Safety (per burner):
- Hardware watchdog timer (<1 second)
- Flame loss detection (<200ms response)
- Local emergency stop capability
- Fail-safe valve closure on power loss

// Master Board Safety (system-wide):
- Global emergency stop monitoring
- Network health monitoring
- Automatic slave timeout detection
- System-wide shutdown capability
- Event logging to SD card

// Mobile App Safety:
- Connection loss alerts
- Emergency stop always visible
- Multi-burner status at a glance
- Audio/visual alarms
- Offline operation capability
```

---

## 📋 CURRENT STATUS & COMPLETED WORK

### **✅ COMPLETED:**
- [x] Hardware architecture design (master-slave)
- [x] Component selection and sourcing plan
- [x] Communication protocol specification
- [x] ESP32-C6 board selection and validation
- [x] Power supply design for both master/slave
- [x] Safety system requirements definition
- [x] Mobile app wireframe and user flow
- [x] Auto-discovery mechanism design
- [x] Scalability planning (1-40 burners)

---

## 🎯 UPCOMING DEVELOPMENT TIMELINE

### **Week 1-2: Master Controller Development**
- [ ] Order and receive ESP32-C6 master components
- [ ] Build master controller breadboard prototype
- [ ] Implement ESP-NOW mesh coordinator firmware
- [ ] Test OLED display and status LED functionality
- [ ] Set up SD card logging system
- [ ] Implement BLE service for mobile communication

### **Week 3-4: Slave Burner Development**
- [ ] Order and receive ESP32-C6 slave components
- [ ] Build first slave burner prototype
- [ ] Implement ESP-NOW client firmware
- [ ] Test relay control and flame sensing
- [ ] Implement auto-discovery and registration
- [ ] Test master-slave communication protocol

### **Week 5-6: Mobile App Development**
- [ ] Set up Flutter project with BLE dependencies
- [ ] Implement master device discovery and connection
- [ ] Create multi-burner dashboard UI
- [ ] Add individual burner control screens
- [ ] Implement emergency stop functionality
- [ ] Add device management (add/remove/rename)

### **Week 7-8: System Integration**
- [ ] Test complete system with 2-3 slave burners
- [ ] Verify mesh network stability and range
- [ ] Test auto-discovery with new devices
- [ ] Calibrate flame detection thresholds
- [ ] Verify emergency stop timing (<200ms)
- [ ] Test mobile app with multiple burners

### **Week 9-10: Scalability Testing**
- [ ] Test with 10+ slave burner simulators
- [ ] Verify mesh network performance at scale
- [ ] Test device addition/removal scenarios
- [ ] Load test mobile app with 40 burner display
- [ ] Network reliability testing
- [ ] Range testing in industrial environment

### **Week 11-12: Production Preparation**
- [ ] Design custom PCBs for master and slave
- [ ] Create weatherproof enclosures
- [ ] Final safety validation and certification
- [ ] User manual and installation guide
- [ ] Mobile app store submission
- [ ] Bulk component sourcing for production

---

## 💡 KEY ADVANTAGES OF THIS APPROACH

### **1. True Scalability**
```
✅ Start with 1 master + 1 burner
✅ Add burners one at a time as needed
✅ No network reconfiguration required
✅ Each burner is independent yet coordinated
✅ Future-proof for expansion up to 40 units
```

### **2. Professional Installation Experience**
```
✅ Plug-and-play slave burner installation
✅ Auto-discovery eliminates manual setup
✅ Single mobile app manages entire system
✅ Visual confirmation of each burner status
✅ Centralized control with individual override
```

### **3. Industrial-Grade Reliability**
```
✅ Mesh network self-heals if nodes fail
✅ Master continues operating if mobile disconnects
✅ Each burner has independent safety systems
✅ System-wide emergency stop capability
✅ Event logging for maintenance and debugging
```

---

**🔥 Bottom Line: This ESP32-C6 master-slave architecture provides a truly scalable, professional gas burner control system that grows with your business needs while maintaining the highest safety standards and user experience!**
