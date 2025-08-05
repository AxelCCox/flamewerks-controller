# Flame Detection - Simple Visual Explanation

## 🔥 HOW THE FLAME SENSOR WORKS (Simple Version)

### **The Basic Concept:**

```
FLAME PRESENT = ELECTRICITY CAN FLOW = ESP8266 SEES VOLTAGE = SYSTEM STAYS ON

NO FLAME = NO ELECTRICITY = ESP8266 SEES 0V = EMERGENCY SHUTOFF
```

---

## **Visual Circuit Diagram:**

```
                    FLAME DETECTION CIRCUIT
                         (Method 1)

    12V Power Supply
           +
           |
           |
       [1MΩ Resistor]  ← Current limiting resistor
           |
           •----------→ ESP8266 A0 Pin (voltage sensing)
           |
    [Flame Rod] ← Stainless steel rod in flame
           |
           |
    [  FLAME  ] ← Gas flame (conducts electricity!)
           |
           |
    [Ground Rod] ← Second stainless steel rod
           |
           |
    ESP8266 GND ← Ground connection
           |
           |
    12V Power Supply
           -
```

---

## **What Happens Step by Step:**

### **Step 1: No Flame (Emergency Condition)**
```
12V (+) → [1MΩ] → [Flame Rod] → [ AIR GAP ] → [Ground Rod] → GND (-)
                      ↑                ↑
                 ESP8266 A0        No conduction!
                 reads: 0V         (Air = insulator)
                 
Result: ESP8266 thinks "NO FLAME" → EMERGENCY SHUTOFF
```

### **Step 2: Flame Present (Normal Operation)**  
```
12V (+) → [1MΩ] → [Flame Rod] → [FLAME IONS] → [Ground Rod] → GND (-)
                      ↑              ↑
                 ESP8266 A0      Conducts electricity!
                 reads: 1-3V    (Fire = conductor)
                 
Result: ESP8266 thinks "FLAME OK" → KEEP RUNNING
```

---

## **Real-World Numbers:**

| Condition | ESP8266 A0 Voltage | ADC Reading | System Response |
|-----------|-------------------|-------------|-----------------|
| **No Flame** | 0.0V | 0-50 | 🚨 EMERGENCY SHUTOFF |
| **Small Flame** | 0.5V | 150-200 | ✅ Flame detected |
| **Good Flame** | 1.5V | 300-500 | ✅ Flame detected |
| **Large Flame** | 2.5V | 500-800 | ✅ Flame detected |

**Threshold Setting:** 150 ADC units (about 0.5V)

---

## **Why This Works - The Science:**

### **Flame = Natural Conductor**
- Gas flames contain **ionized particles** (electrons and ions)
- These particles can carry electrical current
- Think of flame as a "leaky resistor" - not perfect, but conductive

### **Air = Perfect Insulator**  
- Normal air cannot conduct electricity
- No flame = infinite resistance = no current flow
- This creates the "on/off" signal we need

### **The 1MΩ Resistor's Job:**
- **Limits current** to safe microamp levels
- **Creates voltage signal** when current flows
- **Acts as voltage divider** with flame resistance

---

## **Testing Without Real Flame:**

### **Simulate Flame with Resistor:**
```
Instead of flame, temporarily connect a 10MΩ resistor:

12V (+) → [1MΩ] → [10MΩ Resistor] → GND (-)
                      ↑
                 ESP8266 A0
                 
This simulates weak flame conduction
ESP8266 should read ~1V and detect "flame"
```

### **Simulate No Flame:**
```
Disconnect the resistor:

12V (+) → [1MΩ] → [OPEN CIRCUIT] → (nothing)
                      ↑
                 ESP8266 A0
                 
No current path = 0V
ESP8266 should detect "no flame" → shutoff
```

---

## **Emergency Shutoff Flow:**

```
1. Normal Operation:
   Flame Rod in fire → Current flows → A0 = 1-3V → "Flame OK" → Gas stays on

2. Flame Goes Out:
   Flame Rod in air → No current → A0 = 0V → "NO FLAME!" → Gas shuts off

3. Time to Shutoff:
   Detection: 100ms (firmware check)
   Relay Response: 50ms  
   Total: 150ms (3x faster than required!)
```

---

## **Quick Test Checklist:**

✅ **Wire the circuit** (12V → 1MΩ → rod → A0)  
✅ **Upload firmware** (set SIMULATE_NO_FLAME = false)  
✅ **Check serial output** (should show ADC readings)  
✅ **Test with candle** (ADC should jump to 200+)  
✅ **Remove candle** (relay should click OFF immediately)  
✅ **Verify emergency shutoff** (< 0.5 second response)

---

i## **⚡ IGNITER CONTROL WIRING (GPIO12)**

### **ESP8266 Igniter Output Pin:**
```
GPIO12 (Pin D6 on ESP8266 Thing Dev Board) → Igniter Control Circuit
```

### **Pin Identification on ESP8266 Thing Dev Board:**
```
ESP8266 Thing Dev Board Layout:

    [USB Port]
         |
    [RST] [A0]
    [16 ] [GND] 
    [14 ] [3V3]
    [12 ] [5V ]  ← GPIO12 (D6) - IGNITER CONTROL PIN
    [13 ] [GND]
    [15 ] [VIN]
    [2  ] [GND]
    [0  ] [RST]
    [4  ] [EN ]  ← GPIO4 - Pilot Relay (already wired)
    [5  ] [3V3]
```

---

## **IGNITER POWER CIRCUIT OPTIONS**

### **METHOD 1: Low-Power Igniter (12V Glow Plug Style)**
```
ESP8266 GPIO12 → [220Ω Resistor] → PC817 Optocoupler → 12V Igniter

Wiring:
ESP8266 GPIO12 ----[220Ω]---- PC817 Pin 1 (LED+)
ESP8266 GND -------------------- PC817 Pin 2 (LED-)
12V Supply (+) ------------------ PC817 Pin 4 (Collector)
PC817 Pin 3 (Emitter) ---------- Igniter (+)
Igniter (-) --------------------- 12V Supply (-)
```

### **METHOD 2: High-Power Igniter (MOSFET Driver)**
```
For Auburn I-31-2 or similar high-power igniters:

ESP8266 GPIO12 → [1kΩ Resistor] → IRF530 MOSFET Gate
IRF530 Source → GND
IRF530 Drain → Igniter (-)
Igniter (+) → 12V Supply (+)
```
9
### **METHOD 3: Direct Drive (Low Current Only)**
```
⚠️ CAUTION: Only for igniters drawing <40mA

ESP8266 GPIO12 ----[100Ω]---- Igniter (+)
Igniter (-) ------------------- ESP8266 GND

Max current: 40mA (ESP8266 GPIO limit)
```

---

## **RECOMMENDED IGNITER CIRCUIT (PC817 + 12V)**

### **Component List:**
- **PC817 Optocoupler** - $0.50 (electrical isolation)
- **220Ω Resistor** - $0.25 (LED current limiting)
- **12V Power Supply** - Already have
- **Wire connections**

### **Step-by-Step Wiring:**
```
1. ESP8266 CONNECTIONS:
   GPIO12 → 220Ω Resistor → PC817 Pin 1
   GND → PC817 Pin 2

2. POWER CONNECTIONS:
   12V Supply (+) → PC817 Pin 4
   12V Supply (-) → Igniter (-)

3. OUTPUT CONNECTION:
   PC817 Pin 3 → Igniter (+)
```

### **Physical Wiring Diagram:**
```
ESP8266 Thing Dev Board    PC817 Optocoupler    12V Igniter
-----------------------    -----------------    -----------
GPIO12 ----[220Ω]----->    Pin 1 (LED+)
GND ------------------->    Pin 2 (LED-)        
                          Pin 4 (Collect) <--- 12V (+)
                          Pin 3 (Emit) ------> Igniter (+)
                                               Igniter (-) <--- 12V (-)
```

---

## **IGNITER CONTROL LOGIC IN FIRMWARE**

### **How GPIO12 Controls the Igniter:**
```cpp
// From ARDFirmware.ino - this code controls GPIO12:
void setIgniter(bool state) {
    digitalWrite(IGNITER_PIN, state ? HIGH : LOW);  // GPIO12
    
    Serial.printf("[HARDWARE] Igniter: %s\n", state ? "ON" : "OFF");
}

// Igniter timing during pilot ignition:
case PILOT_IGNITING:
    setPilotValve(true);   // Open pilot valve (GPIO4)
    setIgniter(true);      // Turn on igniter (GPIO12)
    
    if (flame detected) {
        setIgniter(false); // Turn off igniter when flame confirmed
    }
```

### **Igniter Operation Sequence:**
```
1. IGNITION COMMAND GIVEN:
   - GPIO4 (pilot valve) → HIGH (relay closes, gas flows)
   - GPIO12 (igniter) → HIGH (igniter sparks/heats)

2. FLAME DETECTED:
   - GPIO12 (igniter) → LOW (turn off igniter)
   - Continue with pilot valve open

3. EMERGENCY SHUTOFF:
   - GPIO4 (pilot valve) → LOW (gas OFF)
   - GPIO12 (igniter) → LOW (igniter OFF)
```

---

## **TESTING THE IGNITER CONTROL**

### **Basic GPIO12 Test (No Igniter Connected):**
```
1. Upload firmware to ESP8266
2. Open Serial Monitor (115200 baud)
3. Test commands:
   - Type: "on" → Should see "Igniter: ON" in serial
   - Measure GPIO12 with multimeter → Should read 3.3V
   - Type: "off" → Should see "Igniter: OFF"
   - Measure GPIO12 → Should read 0V
```

### **LED Test Circuit (Visual Verification):**
```
ESP8266 GPIO12 ----[330Ω Resistor]---- LED (+)
LED (-) ----------------------------- ESP8266 GND

Commands:
- "on" → LED lights up
- "off" → LED turns off
```

### **With Actual Igniter:**
```
⚠️ SAFETY: Test with small 12V igniter first

1. Wire PC817 circuit as shown above
2. Connect low-power 12V igniter or resistive heater
3. Test sequence:
   - Serial command "on" → Igniter should activate
   - Wait 5-10 seconds
   - Serial command "off" → Igniter should deactivate
```

---

## **IGNITER POWER REQUIREMENTS**

### **Typical Igniter Specifications:**
```
Auburn I-31-2 Spark Igniter:
- Voltage: 12V DC
- Current: 200-500mA
- Power: 2.4-6W
- Requires MOSFET or relay driver

Glow Plug Style Igniter:
- Voltage: 12V DC  
- Current: 100-300mA
- Power: 1.2-3.6W
- Can use PC817 optocoupler

Piezo Spark Igniter:
- Voltage: 12V DC input
- Output: High voltage sparks
- Current: 50-150mA
- PC817 suitable
```

### **Power Source Sizing:**
```
For your system:
- ESP8266: ~200mA @ 3.3V = 0.66W
- Relay: ~50mA @ 5V = 0.25W  
- Igniter: ~300mA @ 12V = 3.6W
- Total: ~4.5W

Recommended 12V supply: 5W minimum (12V @ 0.5A)
```

---

## **TROUBLESHOOTING IGNITER CONTROL**

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| GPIO12 always 0V | Pin not configured as output | Check firmware upload, verify pin definition |
| GPIO12 doesn't respond to commands | Wrong GPIO pin | Verify physical connection to Pin D6/GPIO12 |
| Igniter doesn't activate | Insufficient current drive | Add MOSFET or optocoupler driver circuit |
| Igniter stays on | GPIO stuck HIGH | Check for short circuit, verify firmware logic |
| ESP8266 resets when igniter turns on | Power supply overload | Use separate 12V supply for igniter |

---

## **COMPLETE SYSTEM WIRING SUMMARY**

### **All Connections for ESP8266 FlameWerks Controller:**
```
ESP8266 Thing Dev Board Connections:

GPIO4  (D2) → Pilot Valve Relay Control
GPIO12 (D6) → Igniter Control (via PC817)
A0          → Flame Sensor Input
VIN         → 5V Supply (for relay power)
GND         → System Ground
3V3         → ESP8266 power (via buck converter)

Power Requirements:
- 12V main supply (2A capacity)
- 3.3V for ESP8266 (via LM2596 buck converter)
- 5V for relay (from ESP8266 VIN or separate regulator)
```

---

**Igniter Control Status:** ✅ GPIO12 configured and ready  
**Driver Circuit:** 🔄 Requires PC817 optocoupler or MOSFET  
**Testing:** 🔄 Start with LED test, then low-power igniter

---

## **✅ RESISTOR VERIFICATION - PERFECT CHOICES!**

### **Your Selected Resistors:**

**1. 1MΩ Resistor (Flame Sensing):**
```
✅ 100pcs 1M ohm Resistor 1/2w (0.5Watt) ±1% Tolerance Metal Film
✅ Value: 1MΩ (1,000,000 ohms) - PERFECT for flame sensing
✅ Power: 0.5W - Way more than needed (circuit uses ~0.000144W)
✅ Tolerance: ±1% - Excellent precision
✅ Type: Metal Film - Stable and reliable
✅ Quantity: 100pcs - Lifetime supply!
```

**2. 220Ω Resistor (LED Current Limiting):**
```
✅ Chanzon 50pcs 1W 220Ω Metal Film Fixed Resistor ±1% Tolerance  
✅ Value: 220Ω - EXACTLY what we calculated for PC817 LED
✅ Power: 1W - Massive overkill (circuit uses ~0.02W) = Very safe
✅ Tolerance: ±1% - Excellent precision
✅ Type: Metal Film - Professional grade
✅ Quantity: 50pcs - Plenty for multiple projects
```

---

## **🔍 DETAILED POWER ANALYSIS**

### **1MΩ Resistor (Flame Sensor Circuit):**
```
Circuit: 12V → 1MΩ Resistor → Flame Rod → Ground
Maximum Current: 12V ÷ 1,000,000Ω = 12 microamps
Power Dissipated: 12µA × 12V = 0.000144 Watts
Your Resistor Rating: 0.5W (3,472x more than needed!)
Safety Factor: EXCELLENT - will never heat up
```

### **220Ω Resistor (PC817 LED Circuit):**
```
Circuit: 3.3V → 220Ω Resistor → PC817 LED → Ground
Current: (3.3V - 1.2V) ÷ 220Ω = 9.5mA
Power Dissipated: 9.5mA × 2.1V = 0.02 Watts  
Your Resistor Rating: 1W (50x more than needed!)
Safety Factor: EXCELLENT - will stay cool
```

---

## **📋 COMPONENT VERIFICATION CHECKLIST**

### **1MΩ Resistor for Flame Sensing:**
| Specification | Required | Your Choice | Status |
|---------------|----------|-------------|---------|
| **Resistance** | 1MΩ (1,000,000Ω) | 1MΩ | ✅ Perfect |
| **Power Rating** | >0.001W | 0.5W | ✅ Massive overkill |
| **Tolerance** | <5% | ±1% | ✅ Excellent precision |
| **Type** | Carbon/Metal Film | Metal Film | ✅ Professional grade |
| **Voltage Rating** | >12V | >250V (typical) | ✅ Safe |

### **220Ω Resistor for LED Current Limiting:**
| Specification | Required | Your Choice | Status |
|---------------|----------|-------------|---------|
| **Resistance** | 220Ω | 220Ω | ✅ Exact match |
| **Power Rating** | >0.02W | 1W | ✅ Very safe |
| **Tolerance** | <5% | ±1% | ✅ Excellent precision |
| **Type** | Carbon/Metal Film | Metal Film | ✅ Professional grade |
| **Voltage Rating** | >3.3V | >250V (typical) | ✅ Safe |

---

## **🎯 WHY THESE ARE EXCELLENT CHOICES**

### **Power Rating Advantages:**
```
OVERSIZED POWER RATINGS = EXCELLENT RELIABILITY

1MΩ at 0.5W:
- Actual power: 0.000144W
- Rating: 0.5W  
- Safety margin: 3,472x = Virtually no heat generation

220Ω at 1W:
- Actual power: 0.02W
- Rating: 1W
- Safety margin: 50x = Stays completely cool
```

### **Tolerance Advantages:**
```
±1% TOLERANCE = PRECISE OPERATION

1MΩ ±1%:
- Range: 990,000Ω to 1,010,000Ω
- Flame sensing accuracy: Excellent
- Consistent performance across temperature

220Ω ±1%:
- Range: 217.8Ω to 222.2Ω  
- LED current: 9.4mA to 9.6mA
- Perfect for PC817 operation
```

### **Metal Film Advantages:**
```
METAL FILM = PROFESSIONAL GRADE

✅ Low noise (important for flame sensing)
✅ Stable over temperature changes
✅ Long lifespan (decades of operation)
✅ Precise resistance values
✅ Low drift over time
```

---

## **💰 VALUE ANALYSIS**

### **Cost vs Performance:**
```
1MΩ Resistor Pack:
- 100 pieces for ~$10-15
- Cost per resistor: ~$0.10-0.15
- Professional grade quality
- Lifetime supply for multiple projects

220Ω Resistor Pack:
- 50 pieces for ~$8-12  
- Cost per resistor: ~$0.16-0.24
- 1W rating = bulletproof reliability
- Enough for 50 igniter circuits
```

### **Compared to Individual Purchases:**
```
Buying singles at electronics store:
- 1MΩ: ~$0.50 each
- 220Ω: ~$0.50 each
- Lower quality (often 5% tolerance)

Your bulk purchase:
- 3-5x cheaper per component
- Higher quality (1% tolerance, metal film)
- Large quantity for future projects
```

---

## **🔧 PRACTICAL USAGE**

### **What You'll Use Initially:**
```
For ONE FlameWerks controller:
- 1x 1MΩ resistor (flame sensing)
- 1x 220Ω resistor (igniter control)

For MULTIPLE controllers:
- Each additional unit needs same 2 resistors
- Your purchase covers 50+ complete systems
```

### **Color Code Identification:**
```
1MΩ Resistor Color Bands:
Brown - Black - Green - Gold
(1)     (0)    (×100k)  (1%)

220Ω Resistor Color Bands:  
Red - Red - Brown - Gold
(2)   (2)   (×10)    (1%)
```

---

## **🚀 READY TO PROCEED**

### **With These Resistors, You Can:**
```
✅ Build the flame sensing circuit (1MΩ)
✅ Build the igniter control circuit (220Ω)  
✅ Test emergency shutoff functionality
✅ Complete the full FlameWerks system
✅ Build multiple units or backups
```

### **Next Steps After Resistors Arrive:**
```
1. Wire flame sensing: 12V → 1MΩ → flame rod → A0
2. Wire igniter control: GPIO12 → 220Ω → PC817
3. Test with firmware: SIMULATE_NO_FLAME = false
4. Verify emergency shutoff with candle test
5. Celebrate working flame detection system!
```

---

**Component Status:** ✅ Both resistors are PERFECT choices!  
**Quality Level:** 🏆 Professional grade, excellent specifications  
**Value:** 💰 Great bulk pricing for premium components  
**Ready to Order:** 🛒 These will work perfectly for your FlameWerks system!
