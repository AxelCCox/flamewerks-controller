FlamewerksDevPlan

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

---

# ESP8266 Hardware Wiring Instructions

## Overview
This section provides detailed, step-by-step instructions for wiring the ESP8266-based FlameWerks burner control system. Follow these instructions carefully to ensure safe and proper operation.

## ⚠️ SAFETY WARNINGS

**ELECTRICAL SAFETY:**
- Turn OFF all power before making any connections
- Use proper electrical isolation - never connect high voltage directly to ESP8266
- Install appropriate fuses and circuit breakers
- Use proper gauge wire for current loads
- Ensure all connections are secure and properly insulated

**GAS SAFETY:**
- Install gas leak detection before testing
- Test all connections with soapy water solution
- Ensure proper ventilation during installation and testing
- Have fire extinguisher readily available
- Follow all local gas codes and regulations

## Required Tools
- Multimeter with microamp measurement capability
- Wire strippers and crimpers
- Screwdrivers (Phillips and flathead)
- Soldering iron and solder (60/40 rosin core)
- Heat shrink tubing
- Electrical tape
- Cable ties
- Drill with appropriate bits for enclosure mounting

## Component Preparation

### ESP8266 Thing Dev Board Pin Assignments
```
Pin D1 (GPIO5)  → Igniter Control (via MOSFET)
Pin D2 (GPIO4)  → Main Valve Relay Control (via optocoupler)
Pin D4 (GPIO2)  → Pilot Valve Relay Control (via optocoupler)
Pin A0          → Flame Sensor Input (0-3.3V)
Pin 3V3         → 3.3V Power Output
Pin GND         → System Ground
Pin VIN         → 5V Power Input (when using USB power)
```

## Step 1: Power Supply Wiring

### 1.1 Main 12V Power Supply
```
12V Supply (+) → Fuse (2A) → Main Bus (+12V)
12V Supply (-) → Main Bus (GND)
```

### 1.2 ESP8266 Power (3.3V)
**Option A: Using LM2596 Buck Converter (Recommended)**
```
+12V Bus → LM2596 Input (+)
GND Bus  → LM2596 Input (-)
LM2596 Output (+) → ESP8266 3V3 Pin
LM2596 Output (-) → ESP8266 GND Pin
```
**Adjust LM2596 output to exactly 3.3V using multimeter**

**Option B: Using USB Power (Testing Only)**
```
USB Cable → ESP8266 Micro-USB Port
(Use only for bench testing, not field deployment)
```

### 1.3 5V Rail for Relays
```
+12V Bus → LM7805 Input (+)
GND Bus  → LM7805 Input (-)
LM7805 Output (+) → +5V Bus
LM7805 Output (-) → GND Bus
```
**Add heat sink to LM7805 if using multiple relays**

## Step 2: Relay Control Circuits

### 2.1 Pilot Valve Relay (Pin D4/GPIO2)
```
ESP8266 GPIO2 → 220Ω Resistor → PC817 Pin 1 (Anode)
ESP8266 GND   → PC817 Pin 2 (Cathode)
+5V Bus       → PC817 Pin 4 (Collector)
PC817 Pin 3   → Beefcake Relay Control Input
GND Bus       → Beefcake Relay Ground
```

### 2.2 Main Valve Relay (Pin D2/GPIO4)
```
ESP8266 GPIO4 → 220Ω Resistor → PC817 Pin 1 (Anode)
ESP8266 GND   → PC817 Pin 2 (Cathode)
+5V Bus       → PC817 Pin 4 (Collector)
PC817 Pin 3   → Beefcake Relay Control Input
GND Bus       → Beefcake Relay Ground
```

### 2.3 Relay Power Connections
```
Pilot Valve Relay:
  +12V Bus → Relay Coil (+)
  GND Bus  → Relay Coil (-)
  
Main Valve Relay:
  +12V Bus → Relay Coil (+)
  GND Bus  → Relay Coil (-)
```

## Step 3: Igniter Control Circuit

### 3.1 MOSFET Switching Circuit
```
ESP8266 GPIO5 → 10kΩ Resistor → IRF530 Gate
GND Bus       → IRF530 Source
+12V Bus      → Auburn I-31-2 Power Input (+)
IRF530 Drain  → Auburn I-31-2 Power Input (-)
```

### 3.2 Igniter Connections
```
Auburn I-31-2 High Voltage Output → Spark Plug Center Electrode
Auburn I-31-2 Ground Terminal     → Spark Plug Ground Electrode
```

## Step 4: Flame Detection Circuit

### 4.1 LM358 Transimpedance Amplifier
```
Component Placement:
  R1 = 1MΩ (Feedback resistor)
  R2 = 100kΩ (Input bias)
  R3 = 10kΩ (Output pull-down)
  C1 = 0.1µF (Power decoupling)
  C2 = 10µF (Output filtering)
```

### 4.2 Circuit Connections
```
LM358 Pin 8 (+V)  → +5V Bus
LM358 Pin 4 (-V)  → GND Bus
LM358 Pin 1 (Out) → R3 (10kΩ) → ESP8266 Pin A0
LM358 Pin 2 (-)   → R1 (1MΩ) → LM358 Pin 1
LM358 Pin 3 (+)   → R2 (100kΩ) → +5V Bus
LM358 Pin 3 (+)   → C2 (10µF) → GND Bus

Flame Sensor Rod → LM358 Pin 2 (-)
Spark Plug Ground → GND Bus
```

### 4.3 Flame Sensor Rod Installation
```
Stainless Steel Rod (1/8" × 4-6")
Position: 1/4" to 1/2" from flame path
Insulation: Use ceramic insulators to prevent ground contact
Connection: Use shielded wire to reduce electrical noise
```

## Step 5: Gas Valve Connections

### 5.1 Pilot Valve Wiring
```
Pilot Valve Coil (+) → Pilot Relay Common (C)
Pilot Valve Coil (-) → Gas System Ground
+12V Gas Supply     → Pilot Relay Normally Open (NO)
```

### 5.2 Main Valve Wiring
```
Main Valve Coil (+) → Main Relay Common (C)
Main Valve Coil (-) → Gas System Ground
+12V Gas Supply    → Main Relay Normally Open (NO)
```

## Step 6: Protection Circuits

### 6.1 TVS Diode Protection
```
12V Supply Line: 1.5KE12A TVS diode across +12V and GND
5V Supply Line:  1.5KE5.0A TVS diode across +5V and GND
3.3V Supply:     1.5KE3.3A TVS diode across +3.3V and GND
GPIO Lines:      Small signal TVS on each control line
```

### 6.2 Fuse Protection
```
Main 12V Input:  3A Fast-Blow Fuse
Relay Circuits:  1A Fast-Blow Fuse each
Igniter Circuit: 2A Fast-Blow Fuse
```

## Step 7: Enclosure Assembly

### 7.1 NEMA 4X Enclosure Preparation
```
Drill Cable Gland Holes:
  - 8 × PG13.5 holes for:
    * Main power input
    * Gas valve connections (2)
    * Igniter high voltage output
    * Flame sensor cable
    * WiFi antenna (if external)
    * Spare connections (2)
```

### 7.2 DIN Rail Installation
```
Mount DIN Rail horizontally inside enclosure
Install terminal blocks for:
  - +12V Bus (Red terminals)
  - +5V Bus (Orange terminals)
  - +3.3V Bus (Yellow terminals)
  - GND Bus (Black terminals)
  - Signal connections (Blue terminals)
```

### 7.3 Component Mounting
```
ESP8266:      Plastic standoffs on DIN rail mount
Relays:       DIN rail mounting clips
Power Supply: Internal mounting brackets
PCB Circuits: Plastic standoffs or PCB rails
```

## Step 8: Grounding System

### 8.1 Star Ground Configuration
```
Main Ground Point: Large terminal block or bus bar
Connect to main ground point:
  - Power supply negative
  - ESP8266 GND
  - All relay grounds
  - Circuit board grounds
  - Enclosure ground lug
```

### 8.2 Earth Ground Connection
```
12AWG Green Wire: Main ground point → Earth ground rod
Ring Lugs:        Both ends properly crimped
Ground Rod:       8-foot copper rod driven into earth
Resistance:       <25 ohms to earth (test with ground meter)
```

## Step 9: Cable Management

### 9.1 Internal Wiring
```
Power Cables:     14-16 AWG stranded for 12V circuits
Control Cables:   18-22 AWG stranded for signals
Shielded Cable:   For flame sensor (RG-174 or similar)
Heat Resistance:  105°C rated wire minimum
Color Coding:     Red(+12V), Orange(+5V), Yellow(+3.3V), Black(GND)
```

### 9.2 External Connections
```
Cable Glands:     PG13.5 with proper sealing washers
Strain Relief:    Internal cable clamps
Weather Sealing:  Silicone sealant around all penetrations
Labeling:         Permanent labels on all external cables
```

## Step 10: Testing and Validation

### 10.1 Power-On Tests (NO GAS CONNECTED)
```
1. Verify all supply voltages with multimeter:
   - 12V ± 0.5V
   - 5V ± 0.25V  
   - 3.3V ± 0.1V

2. Test ESP8266 programming and WiFi:
   - Upload firmware via Arduino IDE
   - Connect to serial monitor (115200 baud)
   - Verify WiFi connection and web interface

3. Test relay operation:
   - Send "ON" command via serial or web
   - Verify relay clicking sounds
   - Measure 12V across relay outputs with multimeter
```

### 10.2 Signal Tests
```
1. Flame sensor circuit:
   - Measure 0V at ESP8266 A0 with no flame simulation
   - Inject 1-5µA test current, verify voltage increase
   - Test for noise immunity near igniter operation

2. Igniter circuit:
   - Verify spark generation at electrodes
   - Check timing: 5-second maximum operation
   - Test emergency shutoff response
```

### 10.3 Gas System Tests (WITH PROPER SAFETY PRECAUTIONS)
```
1. Pressure testing:
   - Test all connections with soapy water
   - Verify no leaks at operating pressure
   - Check valve operation with gas flow

2. Ignition testing:
   - Start with small test flame
   - Verify proper ignition sequence
   - Test flame detection accuracy
   - Validate safety shutoffs
```

## Step 11: Final System Validation

### 11.1 Safety System Tests
```
1. Emergency stop response:
   - Verify <500ms shutoff time
   - Test from web interface and serial commands
   - Ensure all valves close immediately

2. Flame loss detection:
   - Simulate flame loss during operation
   - Verify immediate valve closure
   - Test retry logic and fault states

3. Power loss recovery:
   - Remove power during operation
   - Verify safe restart in OFF state
   - Test system integrity after power restoration
```

### 11.2 Network and Interface Tests
```
1. WiFi reliability:
   - Test connection recovery after network outage
   - Verify web interface responsiveness
   - Test multiple simultaneous connections

2. Serial interface:
   - Verify all commands function properly
   - Test status reporting accuracy
   - Validate debug output levels
```

## Troubleshooting Guide

### Common Issues and Solutions

**ESP8266 Won't Connect to WiFi:**
- Verify SSID and password in firmware
- Check 2.4GHz network availability (5GHz not supported)
- Ensure adequate signal strength (-70dBm or better)
- Try different WiFi channel (1, 6, or 11 recommended)

**Relays Not Switching:**
- Check optocoupler connections and orientation
- Verify 220Ω current limiting resistors
- Test 5V supply to relay coils
- Measure GPIO output voltage (should be 3.3V)

**Flame Detection Issues:**
- Check sensor rod position and cleanliness
- Verify LM358 power supply (exactly 5V)
- Test circuit with known current source
- Ensure proper grounding and shielding

**Igniter Not Sparking:**
- Check 12V supply to Auburn I-31-2
- Verify MOSFET switching with oscilloscope
- Test spark plug gap (0.030" typical)
- Ensure proper high voltage connections

**Web Interface Problems:**
- Clear browser cache and cookies
- Check ESP8266 IP address via serial monitor
- Verify network firewall settings
- Test direct IP access vs. mDNS hostname

## Maintenance Schedule

### Weekly Checks
- Visual inspection of all connections
- Test emergency stop function
- Verify flame detection accuracy
- Check WiFi connection status

### Monthly Maintenance  
- Clean flame sensor rod
- Test all safety shutoff functions
- Inspect enclosure sealing
- Verify ground resistance <25 ohms

### Annual Service
- Replace flame sensor if corroded
- Test all electrical connections
- Update firmware if available
- Recalibrate flame detection circuit
- Inspect and replace fuses if needed

## Documentation Requirements

### Installation Records
- Wiring diagram with actual wire colors and terminal numbers
- Component serial numbers and installation dates
- Initial calibration values and test results
- Ground resistance measurements
- Gas pressure test results

### Operating Procedures
- Startup and shutdown sequences
- Emergency procedures
- Troubleshooting contact information
- Maintenance log sheets
- Safety inspection checklists

---

**INSTALLATION COMPLETE**

After following these instructions, your FlameWerks ESP8266 burner control system should be ready for safe operation. Remember to perform all safety tests before putting the system into regular service, and maintain proper documentation for future reference and troubleshooting.

For technical support or questions about this installation, refer to the development plan contact information or consult with a qualified technician familiar with gas burner control systems.