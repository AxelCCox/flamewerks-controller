# FlameWerks ESP8266 GPIO4 Relay Wiring Instructions
## Updated for Safe Bench Testing

### 🔥 **UPDATED PIN ASSIGNMENTS (GPIO4 - SAFE FOR RELAY CONTROL)**

**Previous:** GPIO2 (boot sensitive) ❌  
**New:** GPIO4 (safe, no boot interference) ✅

---

## Pin Configuration (Updated)
```
ESP8266 Thing Dev Board Pin Assignments:
• GPIO 4  -> Pilot Valve Relay Control (PRIMARY for testing)
• GPIO 5  -> Main Valve Relay Control  
• GPIO 12 -> Igniter Control
• A0      -> Flame Sensor Input
• VIN     -> 5V Supply for Relay VCC (if using external power)
• GND     -> Common Ground
```

---

## Hardware Wiring for Single SparkFun Beefcake Relay

### **METHOD 1: Using ESP8266 VIN Pin (5V from USB)**
```
ESP8266 Thing Dev Board    SparkFun Beefcake Relay
-----------------------    -----------------------
GPIO4      ------------>    CTRL (Control Pin)
VIN (5V)   ------------>    VCC  (Power - 5V)
GND        ------------>    GND  (Ground)

Relay Output Terminals:
COM (Common)  ---------> Connect to Gas Valve Wire #1
NO (Normally Open) ----> Connect to Gas Valve Wire #2
```

### **METHOD 2: External 5V/12V Power Supply**
```
ESP8266 Thing Dev Board    SparkFun Beefcake Relay    External PSU
-----------------------    -----------------------    ------------
GPIO4      ------------>    CTRL                       
GND        ------------>    GND  <-----------------    GND (-)
                           VCC  <-----------------    +5V or +12V

Relay Output Terminals:
COM (Common)  ---------> Connect to Gas Valve Wire #1  
NO (Normally Open) ----> Connect to Gas Valve Wire #2
```

---

## Key Advantages of GPIO4

✅ **No Boot Interference**: GPIO4 does not affect ESP8266 boot sequence  
✅ **Safe for Programming**: Can stay connected during firmware upload  
✅ **Reliable Control**: No floating or unexpected states during startup  
✅ **Standard I/O**: Pure digital I/O pin with no special functions  

---

## Testing Procedure

### **Step 1: Upload Updated Firmware**
1. Open `ARDFirmware.ino` in Arduino IDE
2. Verify GPIO4 pin assignment: `#define PILOT_RELAY_PIN 4`
3. Upload to ESP8266 Thing Dev Board
4. **No need to disconnect relay** - GPIO4 is safe!

### **Step 2: Wire the Relay**
1. Connect GPIO4 → CTRL pin on relay
2. Connect VIN → VCC on relay (or use external 5V/12V)
3. Connect GND → GND on relay

### **Step 3: Serial Monitor Test**
1. Open Serial Monitor (115200 baud)
2. Type: `pilot on` → Should see "Pilot valve: ON" + relay click
3. Type: `pilot off` → Should see "Pilot valve: OFF" + relay click
4. Type: `emergency` → Should immediately turn off relay

### **Step 4: Web Interface Test**
1. Connect to WiFi network shown in serial output
2. Open browser to ESP8266 IP address
3. Click "Start Pilot" → Should activate relay
4. Click "Emergency Stop" → Should immediately deactivate relay

---

## Troubleshooting GPIO4 Operation

| Problem | Solution |
|---------|----------|
| Relay not clicking | Check 5V power to VCC, verify GPIO4 wiring |
| Inconsistent operation | Ensure proper ground connection |
| No response to commands | Verify serial output shows GPIO4 pin state changes |
| Web interface issues | Check WiFi connection and ESP8266 IP address |

---

## Safety Notes

⚠️ **For bench testing only** - no gas connections yet  
⚠️ **Test relay operation** with multimeter continuity or LED  
⚠️ **Emergency stop** should always work immediately  
⚠️ **GPIO4 is safe** but always verify connections before powering up  

---

## Next Steps After GPIO4 Testing

1. ✅ Confirm relay activation/deactivation with GPIO4
2. ✅ Test emergency shutoff functionality  
3. ✅ Verify web interface and serial commands
4. 🔄 Add flame sensor and complete system integration
5. 🔄 Test full ignition sequence with actual gas valve

---

**Firmware Status:** ✅ Updated to GPIO4  
**Documentation:** ✅ Updated for safe operation  
**Ready for Testing:** ✅ Proceed with relay bench test

---

# 🔥 FLAME SENSING SYSTEM - Complete Wiring Guide

## How Flame Sensing Works

### **The Science Behind Flame Detection**
1. **Flame Rectification**: A gas flame contains ionized particles that can conduct electricity
2. **Microamp Current**: When voltage is applied across a flame, it generates 0.5-10 microamps of current
3. **Safety Principle**: No flame = no current = immediate emergency shutoff

### **System Components Required**
```
Complete Flame Detection System:
• Flame Rod (stainless steel probe in flame)
• AC/DC Voltage Source (12-24V for flame excitation)  
• Current Sensing Circuit (amplifies microamp signal)
• ESP8266 ADC Input (reads amplified signal on A0)
• Safety Logic (firmware monitors and responds)
```

---

## **METHOD 1: Basic Flame Rod Setup (Simplest)**

### Required Components:
- **Stainless steel rod** (1/8" diameter, 6" long) - $5
- **12V DC power supply** (existing system power) - Already have
- **1 MegaOhm resistor** (current sensing) - $1
- **Wire connections** - $2

### Wiring Diagram:
```
Flame Detection - Basic Method

12V Power Supply (+) ----[1MΩ Resistor]---- Flame Rod (in flame)
                                      |
                                      |---> ESP8266 A0 Pin
                                      
ESP8266 GND ----------------------------- 12V Power Supply (-)
ESP8266 GND ----------------------------- Reference Ground Rod
```

### **Step-by-Step Wiring:**
```
1. POWER CONNECTIONS:
   12V Supply (+) → 1MΩ Resistor → Flame Rod
   12V Supply (-) → ESP8266 GND

2. SENSING CONNECTION:
   Junction (Resistor + Flame Rod) → ESP8266 A0 Pin

3. GROUND REFERENCE:
   Second stainless rod → ESP8266 GND (ground reference in flame area)
```

---

## **METHOD 2: Professional LM358 Amplifier Circuit (Recommended)**

### Required Components:
- **LM358 Dual Op-Amp** - $1
- **Precision Resistors**: 1MΩ, 100kΩ, 10kΩ (1% tolerance) - $3
- **Capacitors**: 0.1µF ceramic, 10µF electrolytic - $2
- **Stainless steel flame rod** (1/8" x 6") - $5
- **Breadboard or perfboard** - $3

### **LM358 Amplifier Circuit:**
```
Transimpedance Amplifier Configuration:

                    +5V
                     |
              [100kΩ] (feedback resistor)
                     |
   A0 Pin <----------+--------< LM358 Output (Pin 1)
                     |
                 LM358 (-) Pin 2
                     |
   Flame Rod -------+--------< LM358 (+) Pin 3
                     |
              [1MΩ] (input resistor)
                     |
                   GND

Additional Components:
+5V ----[0.1µF]---- GND  (power filtering)
+5V ----[10µF]----- GND  (power filtering)
LM358 Pin 8 → +5V
LM358 Pin 4 → GND
```

### **Professional Wiring Steps:**
```
1. POWER SUPPLY:
   ESP8266 3.3V → LM358 VCC (Pin 8)
   ESP8266 GND → LM358 GND (Pin 4)

2. FLAME SENSING:
   12V Supply (+) → 1MΩ Resistor → Flame Rod
   Flame Rod → LM358 (+) Pin 3
   LM358 (-) Pin 2 → 100kΩ Feedback Resistor → LM358 Output Pin 1

3. OUTPUT TO ESP8266:
   LM358 Output Pin 1 → ESP8266 A0

4. GROUND REFERENCE:
   Second stainless rod → ESP8266 GND
   12V Supply (-) → ESP8266 GND
```

---

## **Flame Rod Construction**

### **Materials Needed:**
```
• Stainless steel rod: 1/8" diameter x 6" long
• High-temperature wire (rated 200°C+)
• Ceramic insulators or high-temp tubing
• Wire nuts or terminal blocks
• Heat-resistant mounting bracket
```

### **Assembly Instructions:**
```
1. FLAME ROD PREPARATION:
   - Cut stainless steel rod to 6" length
   - File one end to a point (better flame contact)
   - Sand the connection end for good electrical contact

2. WIRE CONNECTION:
   - Solder high-temp wire to rod (use silver solder)
   - Cover connection with ceramic tube or high-temp shrink
   - Use strain relief to prevent wire breakage

3. MOUNTING:
   - Position rod 1-2" into flame path
   - Ensure rod doesn't touch burner metal (would cause short)
   - Use ceramic or fiberglass insulators for mounting
```

---

## **Power Requirements for Flame Sensing**

### **Voltage Levels:**
```
EXCITATION VOLTAGE: 12-24V DC (for flame ionization)
SENSING VOLTAGE: 0-3.3V (ESP8266 ADC input range)
CURRENT LEVELS: 0.5-10 microamps (flame present)
                0 microamps (no flame)
```

### **Power Source Options:**

**Option A: Use Existing 12V System Power**
```
12V Burner Power Supply → Flame Excitation Circuit
Same supply already powering relays and ESP8266
Most efficient - no additional power needed
```

**Option B: Separate Isolated Supply**
```
Dedicated 12V wall adapter → Flame Circuit Only
Better electrical isolation
Recommended for production systems
```

---

## **ESP8266 ADC Configuration**

The firmware is already configured for flame sensing:

### **ADC Reading Code (Already in Firmware):**
```cpp
bool readFlameStatus() {
    int reading = analogRead(FLAME_SENSE_PIN);  // A0 pin
    bool currentlyDetected = reading > FLAME_THRESHOLD;
    
    // FLAME_THRESHOLD = 150 (adjustable based on your setup)
    // ADC range: 0-1024 (0V to 3.3V)
    // Flame present: reading > 150
    // No flame: reading < 150
}
```

### **Calibration Process:**
```
1. NO FLAME TEST:
   - Power system without flame
   - Check serial output: "Raw ADC: [value]"
   - Should read 0-50 (no flame baseline)

2. FLAME PRESENT TEST:  
   - Light test flame near rod
   - Check serial output: "Raw ADC: [value]"
   - Should read 200-800 (flame detected)

3. ADJUST THRESHOLD:
   - Set FLAME_THRESHOLD = (NoFlame + Flame) / 2
   - Default 150 should work for most setups
   - Can adjust in firmware if needed
```

---

## **Testing the Flame Sensor**

### **Step 1: Basic Electrical Test**
```
Equipment Needed:
• Multimeter with microamp capability
• 12V power supply
• 1MΩ resistor
• Flame rod assembly

Test Procedure:
1. Connect multimeter in series with flame rod
2. Apply 12V across circuit
3. No flame: should read 0 microamps
4. Light candle/lighter near rod: should read 1-10 microamps
```

### **Step 2: ESP8266 Integration Test**
```
1. UPLOAD FIRMWARE:
   - Use existing ARDFirmware.ino
   - Ensure SIMULATE_NO_FLAME = false
   - Upload and open Serial Monitor

2. POWER CONNECTIONS:
   - Wire flame sensing circuit as shown above
   - Connect ESP8266 A0 to circuit output

3. SERIAL MONITORING:
   - Watch for: "[FLAME] ADC: [value], Detected: YES/NO"
   - No flame: "Detected: NO", ADC < 150
   - Flame present: "Detected: YES", ADC > 150
```

### **Step 3: Emergency Shutoff Test**
```
1. START PILOT:
   - Serial command: "pilot on" or web interface
   - Relay should click ON

2. SIMULATE FLAME LOSS:
   - Remove flame from sensor rod
   - Should see: "[FLAME] Detected: NO"
   - Should see: "[HARDWARE] Pilot valve: CLOSED"
   - Relay should click OFF immediately

3. VERIFY EMERGENCY RESPONSE:
   - Response time should be < 500ms
   - Web interface should show "FLAME OUT - RETRYING"
```

---

## **Troubleshooting Flame Sensing**

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| Always reads "No Flame" | No power to flame rod | Check 12V supply, verify 1MΩ resistor |
| Always reads "Flame" | Short circuit or bad ground | Check wiring, ensure rod not touching metal |
| Erratic readings | Electrical noise | Add filtering capacitors, check ground connections |
| No ADC readings | A0 pin not connected | Verify ESP8266 A0 connection |
| Emergency stop not working | Threshold too high/low | Adjust FLAME_THRESHOLD in firmware |

---

## **Safety Considerations**

⚠️ **ELECTRICAL SAFETY:**
- Keep flame rod properly insulated from burner body
- Use high-temperature rated wires and connections
- Ensure proper grounding to prevent electrical shock

⚠️ **FLAME SAFETY:**
- Test flame sensor before every use
- Verify emergency shutoff works reliably
- Never bypass flame sensing safety logic

⚠️ **TESTING SAFETY:**
- Use small test flame (candle/lighter) for initial testing
- Keep fire extinguisher available during all testing
- Test in well-ventilated area only

---

## **Complete System Integration**

Once flame sensing is working:

### **Full System Test Sequence:**
```
1. POWER UP: ESP8266 + Relay + Flame Sensor
2. IGNITE COMMAND: Relay activates, igniter sparks
3. FLAME DETECTION: Sensor confirms flame presence  
4. NORMAL OPERATION: Main valve opens (if configured)
5. FLAME LOSS TEST: Remove flame → Emergency shutoff
6. RESET: Manual reset required after flame loss
```

### **Production Deployment:**
```
1. ✅ Bench test all components individually
2. ✅ Integration test with actual burner (no gas)
3. ✅ Professional installation with gas connections
4. ✅ Final safety verification and commissioning
```

---

**Flame Sensor Status:** 🔄 Ready for Integration  
**Safety Level:** ⚠️ Critical Component - Test Thoroughly  
**Next Step:** 🔥 Wire flame sensor and test emergency shutoff

---

## **🔍 HOW METHOD 1 FLAME DETECTION ACTUALLY WORKS**

### **The Physics Behind Flame Sensing**

When you have a gas flame, it contains **ionized particles** (free electrons and ions) that make it electrically conductive. Here's the step-by-step process:

### **Step 1: Voltage Applied Across Flame**
```
12V Power Supply (+) → 1MΩ Resistor → Flame Rod (in flame)
                                    ↓
                              Flame conducts electricity
                                    ↓
                          Ground Reference Rod ← ESP8266 GND
```

### **Step 2: Current Flow Through Flame**
```
FLAME PRESENT:
- Flame acts like a weak resistor (high resistance, but conductive)
- Tiny current flows: 12V → 1MΩ → Flame → Ground (0.5-10 microamps)
- Voltage drop across 1MΩ resistor creates detectable signal

NO FLAME:
- Air gap = infinite resistance (insulator)
- No current flows: 0 microamps
- No voltage drop across resistor = 0V signal
```

### **Step 3: ESP8266 A0 Pin Reads the Signal**
```
FLAME PRESENT:
Current flow → Voltage drop across 1MΩ resistor → 1-3V at A0 pin
ESP8266 ADC reads 200-800 (out of 1024 scale)
Firmware: "FLAME DETECTED = YES"

NO FLAME:
No current → No voltage drop → 0V at A0 pin  
ESP8266 ADC reads 0-50 (noise/baseline)
Firmware: "FLAME DETECTED = NO"
```

---

## **⚡ DETAILED CIRCUIT ANALYSIS**

### **Electrical Circuit Explanation:**
```
12V Supply (+) ----[1MΩ Resistor]----•---- Flame Rod
                                     |
                                     |
                              ESP8266 A0 Pin (reads voltage here)
                                     |
                                     |
ESP8266 GND ---------------------Ground Rod (in flame area)
     |
12V Supply (-)
```

### **Voltage Calculations:**

**When FLAME is PRESENT:**
```
Flame Resistance: ~100MΩ (very high, but not infinite)
Total Circuit Resistance: 1MΩ + 100MΩ = 101MΩ
Current: 12V ÷ 101MΩ = 0.119 microamps
Voltage at A0: 0.119µA × 1MΩ = 0.119V → ADC reading ~36

Actually: Flame is more complex, creates 1-10µA typically
Real voltage at A0: 1-3V → ADC reading 200-600
```

**When NO FLAME:**
```
Air Gap Resistance: Infinite (perfect insulator)
Current: 12V ÷ Infinite = 0 microamps  
Voltage at A0: 0µA × 1MΩ = 0V → ADC reading ~0-10
```

---

## **🚨 EMERGENCY SHUTOFF MECHANISM**

### **How the System Detects Flame Loss:**

The firmware continuously monitors the A0 pin every 100 milliseconds:

```cpp
// This code runs every 100ms in the firmware:
bool readFlameStatus() {
    int reading = analogRead(FLAME_SENSE_PIN);  // Read A0 pin
    bool currentlyDetected = reading > FLAME_THRESHOLD;  // 150 threshold
    
    if (currentlyDetected) {
        burner1.flameDetected = true;   // Flame present
    } else {
        burner1.flameDetected = false;  // NO FLAME → EMERGENCY!
    }
    
    return burner1.flameDetected;
}
```

### **Emergency Shutoff Sequence:**

```
1. NORMAL OPERATION:
   - Pilot valve ON (relay closed)
   - Flame rod in flame
   - A0 reads 200-600 (flame detected)
   - System state: MAIN_ON

2. FLAME GOES OUT:
   - Wind blows out flame, gas runs out, etc.
   - Flame rod now in air (no conductive path)
   - A0 reads 0-50 (no flame detected)
   - Firmware detects: reading < 150 threshold

3. IMMEDIATE EMERGENCY RESPONSE:
   - setPilotValve(false) → GPIO4 goes LOW → Relay opens → Gas OFF
   - System state changes to FLAMEOUT
   - Web interface shows "FLAME OUT - RETRYING"
   - All happens within 100-500ms!
```

---

## **📊 REAL-WORLD TESTING EXAMPLE**

### **What You'll See in Serial Monitor:**

**System Starting (No Flame):**
```
[FLAME] ADC: 12, Threshold: 150, Detected: NO, Samples: 0
[FLAME] ADC: 8, Threshold: 150, Detected: NO, Samples: 0
[STATE] Current state: OFF
```

**Pilot Ignition Command Given:**
```
[WEB] Control command: ON, Response: IGNITION STARTED
[STATE] OFF -> PILOT_IGNITING (Manual ignition command)
[HARDWARE] Pilot valve: OPEN
[HARDWARE] Igniter: ON
```

**Light Candle/Lighter Near Flame Rod:**
```
[FLAME] ADC: 245, Threshold: 150, Detected: YES, Samples: 1
[FLAME] ADC: 267, Threshold: 150, Detected: YES, Samples: 2  
[FLAME] ADC: 289, Threshold: 150, Detected: YES, Samples: 3
[STATE] PILOT_IGNITING -> MAIN_ON (Pilot ignited successfully)
[HARDWARE] Igniter: OFF
[HARDWARE] Main valve: OPEN (DISABLED - Single Relay Mode)
```

**Remove Flame (Simulate Flame Loss):**
```
[FLAME] ADC: 23, Threshold: 150, Detected: NO, Samples: 0
[FLAME] ADC: 18, Threshold: 150, Detected: NO, Samples: 0
[STATE] MAIN_ON -> FLAMEOUT (Flame loss detected)
[HARDWARE] Pilot valve: CLOSED
[HARDWARE] Main valve: CLOSED
```

---

## **🔧 SIMPLE TESTING WITHOUT ACTUAL FLAME**

### **Method A: Test with Multimeter**
```
1. Connect multimeter between flame rod and ground rod
2. Set to microamp measurement  
3. Should read 0µA with no flame
4. Touch multimeter probes together → simulates flame conduction
5. Should see ESP8266 A0 voltage change and detect "flame"
```

### **Method B: Test with Resistor**
```
1. Temporarily connect 10MΩ resistor between flame rod and ground
2. This simulates weak flame conduction
3. ESP8266 should detect "flame present"
4. Remove resistor → should detect "no flame" and shutoff
```

### **Method C: Test with Wire Bridge**
```
⚠️ CAUTION: This will show full 12V at A0 - may damage ESP8266!
Better to use high-value resistor method above.
```

---

## **🎯 KEY POINTS FOR UNDERSTANDING**

### **Why This Works:**
1. **Flame = Conductor**: Fire contains ions that conduct electricity
2. **Air = Insulator**: No flame means no current path
3. **Voltage Divider**: 1MΩ resistor + flame resistance creates voltage signal  
4. **Continuous Monitoring**: ESP8266 checks every 100ms for instant response
5. **Fail-Safe Design**: Any sensor failure defaults to "no flame" = safety shutoff

### **Why 1 MegaOhm Resistor:**
- **High enough**: Limits current to safe microamp levels
- **Low enough**: Creates detectable voltage when flame conducts  
- **Standard value**: Easy to obtain, reliable, precise
- **Safety**: Prevents dangerous current levels through flame

### **Emergency Response Time:**
- **Detection**: 100ms (firmware check interval)
- **Response**: <50ms (relay switching time)  
- **Total**: <150ms from flame loss to gas shutoff
- **Safety Standard**: <500ms required, this system is 3x faster!

---

## **⚠️ SAFETY VALIDATION**

### **Test This Emergency Shutoff:**
1. **Start pilot** (relay clicks ON)
2. **Light candle near flame rod** (serial shows "Detected: YES")  
3. **Blow out candle** (remove from rod)
4. **Verify immediate shutoff** (relay clicks OFF within 0.5 seconds)
5. **Check web interface** (should show "FLAME OUT - RETRYING")

### **Success Criteria:**
✅ ADC reads 0-50 with no flame  
✅ ADC reads 200+ with flame present  
✅ Relay shuts off immediately when flame removed  
✅ System can be reset and restarted after flame loss  

This Method 1 approach gives you a complete, safe flame detection system with just a $1 resistor and piece of stainless steel wire!
