# FlameWerks Remote Burner Control System

## Project Overview
Remote-control ignition system for FlameWerks propane weed-control burners with safety flame detection and multi-burner coordination.

---

# System Requirements

## Core Hardware
- **ESP8266 Thing Dev Board** - Main controller with Wi-Fi
- **Auburn I-31-2 Spark Plug Igniter** - High-voltage ignition with built-in flame sensing
- **SparkFun Beefcake Relay Boards** (2 per burner) - Pilot and main valve control
- **Flame Rectification Circuit** - Microamp current detection (0.5-10µA)

## Safety Features
- **Immediate shutoff** if ignition fails within 5 seconds
- **Flame loss detection** with emergency valve closure
- **Retry logic** with 3-second cooldown periods (max 3 attempts)
- **Electrical isolation** via optocouplers
- **Fault state management** requiring manual reset

## Multi-Burner Capabilities
- **Group ignition** with 3-second delays between groups of 4
- **Individual control** via touchscreen web interface
- **Drag selection** for batch operations
- **Real-time status monitoring** with visual flame indicators

---

# Next Steps to Complete Project

## Phase 0: Arduino IDE Setup (Complete First!)

### Step 0.1: Install ESP8266 Board Package
**Priority: IMMEDIATE - Required before uploading firmware**

1. **Open Arduino IDE**
   - Go to File → Preferences

2. **Add ESP8266 Board Manager URL:**
   - In "Additional Boards Manager URLs" field, add:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
   - If there are already URLs, separate with commas

3. **Install ESP8266 Board Package:**
   - Go to Tools → Board → Boards Manager
   - Search for "esp8266"
   - Find "esp8266 by ESP8266 Community" 
   - Click Install (latest version, usually 3.x.x)
   - Wait for installation to complete

4. **Select ESP8266 Board:**
   - Go to Tools → Board → ESP8266 Boards
   - Select "SparkFun ESP8266 Thing Dev"
   - Or select "NodeMCU 1.0 (ESP-12E Module)" if Thing Dev not available

5. **Configure Board Settings:**
   - Tools → Flash Size: "4MB (FS:2MB OTA:~1019KB)"
   - Tools → CPU Frequency: "80 MHz"
   - Tools → Upload Speed: "115200"

### Step 0.2: Install Required Libraries
**Install these libraries via Library Manager (Sketch → Include Library → Manage Libraries):**

- [ ] **ESP8266WiFi** (included with board package)
- [ ] **ESP8266WebServer** (included with board package) 
- [ ] **ESP8266mDNS** (included with board package)

*Note: These libraries are automatically included when you install the ESP8266 board package*

### Step 0.3: Test Compilation
1. **Create new sketch and save as "FlameWerkFirmware.ino"**
2. **Copy the firmware code from Firmware.cpp**
3. **Update WiFi credentials:**
   ```cpp
   const char* ssid = "YourWiFiNetwork";
   const char* password = "YourWiFiPassword";
   ```
4. **Verify/Compile (Ctrl+R)** - should compile without errors
5. **Connect ESP8266 via USB and upload (Ctrl+U)**

### Common Arduino IDE Issues:

**If "SparkFun ESP8266 Thing Dev" not in board list:**
- Use "NodeMCU 1.0 (ESP-12E Module)" instead
- Pin assignments remain the same

**If compilation still fails:**
- Restart Arduino IDE after installing board package
- Check that ESP8266 board is selected (not Arduino Uno)
- Verify board package version is 3.0.0 or newer

**If upload fails:**
- Check USB cable connection
- Try different USB port
- Press and hold FLASH button on ESP8266 during upload
- Reduce upload speed to 57600 in Tools menu

## Phase 1: Hardware Procurement & Assembly (Est. 2-3 weeks)

### Step 1.1: Order Components ✅ READY TO ORDER
**Priority: HIGH - All components identified and sourced**

**Electronics Shopping List:**
- [ ] **PC817 Optocouplers** × 3 ($1.50) - Mouser/Digi-Key
- [ ] **LM358 Op-Amp** × 1 ($0.75) - Standard dual op-amp
- [ ] **Precision Resistors** (1%, metal film): 1MΩ, 100kΩ, 10kΩ ($3)
- [ ] **Capacitors**: 0.1µF ceramic, 10µF electrolytic ($2)
- [ ] **IRF530 MOSFET** × 1 ($2) - For igniter switching
- [ ] **220Ω Resistors** × 3 ($1) - Current limiting for optocouplers

**Power Components:**
- [ ] **12V 3A Power Supply** ($20) - Mean Well or equivalent
- [ ] **LM2596 Buck Converter** 12V→3.3V ($5) - Adjustable module
- [ ] **LM7805 Linear Regulator** ($2) - TO-220 package
- [ ] **TVS Diodes** 1.5KE12A × 4 ($4) - Overvoltage protection
- [ ] **Fuses** 2A, 5A fast-blow × 3 ($3)

**Mechanical Components:**
- [ ] **Stainless Steel Rod** 1/8" × 8" ($5) - Flame sensor probe
- [ ] **NEMA 4X Enclosure** 8×6×4" ($25) - Polycarbonate outdoor rated
- [ ] **Cable Glands** PG13.5 × 8 ($16) - Weatherproof entries
- [ ] **DIN Rail** and terminal blocks ($10)
- [ ] **Ground Wire** 12AWG × 10ft + ring lugs ($8)

**Total Per Burner:** ~$107 (vs. original estimate $85-120) ✅

### Step 1.2: Build and Test Flame Detection Circuit
**Status: Ready for hardware - Circuit design complete**

**Implementation Steps:**
1. **Breadboard the LM358 circuit** using the transimpedance configuration
2. **Test with simulated flame current** (1-10µA from function generator)
3. **Calibrate output voltage** to 0-3.3V range for ESP8266 ADC
4. **Validate noise immunity** with igniter sparking nearby
5. **Transfer to perfboard** for permanent installation

**Expected Timeline:** 3-5 days after components arrive

### Step 1.3: Build Control Circuit
**Estimated time: 1 week**

1. **Wire optocoupler isolation:**
   ```
   ESP8266 GPIO → 220Ω → PC817 Pin 1
   ESP8266 GND → PC817 Pin 2
   5V Supply → PC817 Pin 4
   PC817 Pin 3 → Beefcake Relay Control
   ```

2. **Install power regulation:**
   - 12V main supply with fuse protection
   - Buck converter for 3.3V ESP power
   - Linear regulator for clean 5V analog power
   - Star ground configuration

3. **Test relay switching:**
   - Verify 3.3V GPIO can trigger optocoupler
   - Confirm relay contacts switch 12V loads
   - Check for back-EMF protection

## Phase 2: Firmware Integration (Est. 1-2 weeks)

### Step 2.1: Hardware Validation ✅ FIRMWARE READY
**Current status: 95% complete - Ready for hardware testing**

**Firmware Features Implemented:**
- [x] **State machine logic** - OFF → PILOT_IGNITING → MAIN_ON → FLAMEOUT/FAULT
- [x] **Safety timeouts** - 5-second ignition timeout, 3-second cooldown
- [x] **Retry logic** - Maximum 3 attempts before FAULT state
- [x] **Web interface** - Real-time status, remote control, emergency stop
- [x] **Serial debugging** - Complete command set for testing
- [x] **Memory optimization** - PROGMEM usage, efficient string handling

**Hardware Testing Checklist:**
- [ ] **GPIO functionality** - Test relay control pins (2, 4, 5)
- [ ] **ADC calibration** - Measure flame sensor on pin A0
- [ ] **Relay timing** - Verify proper valve sequencing
- [ ] **Igniter control** - Test spark generation timing
- [ ] **Emergency shutoff** - Validate <500ms response time

### Step 2.2: Multi-Burner Expansion
**Current status: 30% complete - Single burner working, expansion designed**

**Implementation Plan:**
1. **Add MCP23017 I2C GPIO expander** - 16 additional pins
2. **Update pin mapping** - Support 4-8 burners per ESP8266
3. **Modify state machine** - Parallel burner operation
4. **Enhance web API** - Group commands with 3-second delays
5. **Test with existing webui.html** - Drag selection integration

**Timeline:** 1 week after single-burner validation

## Phase 3: Safety Testing & Validation (Est. 1 week)

### Step 3.1: Safety System Validation
**Priority: CRITICAL - Cannot deploy without completion**

1. **Flame detection accuracy:**
   - [ ] Test with various flame sizes and positions
   - [ ] Verify detection within 200-500ms
   - [ ] Test false positive rejection (sparks, RF noise)

2. **Emergency shutoff testing:**
   - [ ] Simulate flame loss - verify immediate valve closure
   - [ ] Test sensor disconnect - confirm FAULT state
   - [ ] Power loss recovery - validate safe startup

3. **Retry logic validation:**
   - [ ] Block ignition, verify 3 retry attempts
   - [ ] Confirm 3-second cooldown between retries
   - [ ] Test manual reset from FAULT state

### Step 3.2: System Integration Testing

1. **Multi-burner coordination:**
   - [ ] Test group ignition with 3-second delays
   - [ ] Verify independent operation after ignition
   - [ ] Test emergency stop affecting all burners

2. **Network reliability:**
   - [ ] Test Wi-Fi reconnection after network loss
   - [ ] Validate web UI responsiveness with multiple clients
   - [ ] Test system behavior during firmware updates

## Phase 4: Documentation & Deployment (Est. 1 week)

### Step 4.1: Create Installation Documentation

1. **Circuit diagrams:** (Use visual diagram prompt from development plan)
   - [ ] Complete wiring schematic
   - [ ] Physical layout drawing
   - [ ] System architecture overview

2. **Assembly instructions:**
   - [ ] Step-by-step wiring guide
   - [ ] Enclosure mounting procedures
   - [ ] Safety precautions and warnings

### Step 4.2: User Documentation

1. **Operation manual:**
   - [ ] Web UI user guide
   - [ ] Serial command reference
   - [ ] Troubleshooting procedures

2. **Maintenance procedures:**
   - [ ] Flame sensor cleaning
   - [ ] Calibration adjustments
   - [ ] Component replacement guide

---

# Current Project Status

## ✅ Complete
- **Firmware compilation successful** - Arduino IDE setup working
- **Memory optimization** - HTML moved to PROGMEM, reduced RAM usage
- **Basic single-burner firmware** with state machine and safety logic
- **Web UI integration** - Embedded HTML serves control interface
- **Serial command interface** for testing and debugging
- **Hardware component analysis** and detailed shopping list
- **Multi-burner web UI** with drag selection and group controls

## 🔄 In Progress  
- **Component procurement** - Ready to order electronics
- **Hardware assembly planning** - Circuit designs complete
- **Firmware testing** - Ready for hardware validation

## ⏳ Pending Hardware
- **Flame detection circuit** assembly and calibration
- **Optocoupler isolation** testing with real relays
- **Real-world ignition timing** validation
- **Multi-burner expansion** - GPIO expander integration

## 🚨 Current Memory Usage Analysis
**Compilation Results (ESP8266):**
- **Program Storage:** 332,511 bytes (31% of 1MB flash) ✅ Good
- **RAM Usage:** 35,936 bytes (43% of 80KB RAM) ⚠️ Moderate  
- **IRAM Usage:** 60,807 bytes (92% of 64KB) ⚠️ High but acceptable
- **Optimization:** HTML moved to PROGMEM to reduce RAM pressure

**Next Optimization Steps:**
- [ ] Remove debug strings to reduce flash usage
- [ ] Implement multi-burner support with shared memory
- [ ] Add OTA update capability for field deployment

---

# Component Shopping List (Consolidated)

## Per Burner Unit (~$85-120)
- ESP8266 Thing Dev Board ($15-20)
- Beefcake Relays × 2 ($16 total)
- PC817 Optocouplers × 3 ($1.50 total)
- Flame detection components ($8-15)
- Power regulation modules ($5-10)
- Protection components ($8-15)
- Enclosure and mounting ($15-25)
- Miscellaneous wiring and hardware ($10-15)

## Shared Tools & Equipment
- Multimeter with microamp capability
- Oscilloscope (for noise analysis)
- Function generator (for testing)
- Basic electronics tools

---

# Success Criteria

## Minimum Viable Product
- [ ] Single burner ignites reliably within 5 seconds
- [ ] Flame loss triggers immediate shutoff
- [ ] Web UI provides real-time status and control
- [ ] System recovers safely from power loss

## Full System Goals
- [ ] 4-8 burners operate independently
- [ ] Group ignition with proper delays
- [ ] Drag selection for batch operations
- [ ] Field-deployable weatherproof enclosure

## Safety Requirements
- [ ] <500ms flame loss detection and shutoff
- [ ] Maximum 3 ignition attempts per cycle
- [ ] Fault state requires manual reset
- [ ] All electrical connections properly isolated

**Target Completion: 4-6 weeks from component order**