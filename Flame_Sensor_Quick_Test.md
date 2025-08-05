# Flame Sensor Quick Test Guide

## 🔥 IMMEDIATE TESTING SETUP (Simplest Method)

### What You Need Right Now:
- **1 MegaOhm resistor** (Radio Shack, Amazon, etc.) - $1
- **Stainless steel wire or rod** (any hardware store) - $3  
- **12V power source** (use existing system power)
- **Jumper wires** for connections

### 5-Minute Wiring Setup:
```
SIMPLE FLAME SENSOR WIRING:

12V Power (+) ----[1MΩ Resistor]---- Stainless Steel Rod
                          |
                          +---------> ESP8266 A0 Pin
                          
ESP8266 GND ---------------------------> 12V Power (-)
```

### Testing Steps:
1. **Wire the circuit** as shown above
2. **Update firmware** - Set `SIMULATE_NO_FLAME = false` in code
3. **Upload firmware** and open Serial Monitor  
4. **Test without flame** - Should see "ADC: 0-50, Detected: NO"
5. **Light candle near rod** - Should see "ADC: 200+, Detected: YES"
6. **Test emergency stop** - Remove flame, relay should turn OFF immediately

---

## Expected Serial Output:

### No Flame Present:
```
[FLAME] ADC: 23, Threshold: 150, Detected: NO, Samples: 0
[FLAME] ADC: 31, Threshold: 150, Detected: NO, Samples: 0
```

### Flame Detected:
```
[FLAME] ADC: 245, Threshold: 150, Detected: YES, Samples: 3
[FLAME] ADC: 267, Threshold: 150, Detected: YES, Samples: 3
```

### Emergency Shutoff Triggered:
```
[FLAME] ADC: 28, Threshold: 150, Detected: NO, Samples: 0
[HARDWARE] Pilot valve: CLOSED
[STATE] MAIN_ON -> FLAMEOUT (Flame loss detected)
```

---

## Firmware Settings for Flame Testing

In `ARDFirmware.ino`, change this line:
```cpp
// Change from:
const bool SIMULATE_NO_FLAME = true;   // Testing mode

// Change to:  
const bool SIMULATE_NO_FLAME = false;  // Real flame sensing
```

Then re-upload the firmware.

---

## Quick Troubleshooting:

| Problem | Quick Fix |
|---------|-----------|
| Always shows "NO" flame | Check 12V power, verify 1MΩ resistor |
| Always shows "YES" flame | Check for short circuit, ensure rod isolated |
| No ADC readings in serial | Verify A0 connection to ESP8266 |
| Relay doesn't respond to flame loss | Confirm SIMULATE_NO_FLAME = false |

---

## Safety Test Sequence:

1. ✅ **Power up system** (ESP8266 + relay + flame sensor)
2. ✅ **Start pilot** (`pilot on` command or web interface)  
3. ✅ **Light test flame** near rod (candle, lighter, etc.)
4. ✅ **Confirm flame detection** (serial output shows "YES")
5. ✅ **Remove flame** and verify immediate relay shutoff
6. ✅ **Test emergency stop** via web interface

**🎯 Success Criteria:** Relay turns OFF within 0.5 seconds of flame removal

---

**Status:** 🔄 Ready for immediate testing  
**Time Required:** ⏱️ 15 minutes setup + testing  
**Safety Level:** ⚠️ Use small test flames only
