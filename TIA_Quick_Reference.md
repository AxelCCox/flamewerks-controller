# Quick Reference: TIA Connection Checklist

## Your Wire Colors → Connections

### ✅ Power Connections (Do First)
- **Red wire** (Pin 6) → **3.3V supply**
- **Green wire** (Pin 3) → **GND**  
- **Green wire** (Pin 1) → **GND**

### ✅ Feedback Network (Critical for stability)
- **220kΩ resistor** + **100kΩ resistor** in series
- Connect between **Blue wire (Pin 4)** and **Blue wire (Pin 8)**
- **10pF capacitor** in parallel with the resistors (same two blue wires)

### ✅ S-Probe Connection (From Your Drawing)
- **ELECTRODE ROD** (0.250" terminal) → **High-temp wire** → **100kΩ resistor** → **Blue wire (Pin 8)**
- **GROUND ROD** (welded to bracket) → **Mounting bracket** → **Burner chassis** → **Star ground**

### ✅ Protection Diode (Single BAT54 - Prevents ignition damage)
At the junction where electrode meets Pin 8:
- **BAT54**: Cathode to junction, Anode to GND (protects against negative spikes)

### ✅ Output to ESP32-C6
- **Blue wire (Pin 4)** → **10kΩ resistor** → **ADC node**
- **ADC node** → **ESP32-C6 A1 pin**
- **ADC node** → **0.1µF capacitor** → **GND**

### ✅ Power Decoupling (Near op-amp)
- **0.1µF capacitor**: Pin 6 to Pin 3
- **10µF capacitor**: Pin 6 to Pin 3

## Component Locations in Your Parts

From your S-probe technical drawing and dev plan parts list:
- ✅ **LMP7721MA** - Your main op-amp
- ✅ **220kΩ + 100kΩ** - For 320kΩ feedback resistor  
- ✅ **10pF** - Feedback stability capacitor
- ✅ **100kΩ** - Input series resistor (electrode protection)
- ✅ **1x BAT54** - Single protection diode (you have one)
- ✅ **10kΩ** - Output filter resistor
- ✅ **0.1µF, 10µF** - Filter and decoupling capacitors
- ✅ **S-Probe Assembly** - Professional flame rod with 0.250" terminal

## Quick Test Procedure

1. **Power on**: Check 3.3V at red wire, 0V at green wires
2. **No smoke**: Good! Circuit is wired correctly
3. **Bias check**: Both blue wires should be near 0V (within ±50mV)
4. **Dummy test**: 3.3V through 3.3MΩ into input → should see ~0.32V at output

## Next Steps After TIA Assembly

1. **Mount in enclosure** close to ESP32-C6
2. **Connect flame rod** with high-temp wire and gland
3. **Connect ADC node** to ESP32-C6 A1 pin  
4. **Test with firmware** - sample at 100Hz, apply thresholds
5. **Calibrate with actual flame** - record µA values for pilot/main flames

## Safety Notes

⚠️ **Keep high-impedance traces short** - contamination causes leakage
⚠️ **Clean with IPA after soldering** - flux residue affects performance  
⚠️ **Star ground all returns** - avoid ground loops
⚠️ **Test before connecting to gas system** - verify all safety interlocks
