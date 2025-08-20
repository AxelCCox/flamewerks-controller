# FlameWerks TIA Circuit - Detailed Wiring Diagram & Instructions

[Open printable schematic (SVG)](./TIA_Schematic.svg)

## Your Current Wiring Status
Based on your photo, you have:
- **Pin 1** (IN+) - **Green wire**
- **Pin 3** (V-) - **Green wire**  
- **Pin 4** (VOUT) - **Blue wire**
- **Pin 6** (V+) - **Red wire**
- **Pin 8** (IN-) - **Blue wire**

## Complete TIA Circuit Diagram

```
                                    ESP32-C6
S-Probe Assembly                      A1 Pin
(From Your Drawing)                      ^
                                        |
ELECTRODE ROD                       ADC Node
(0.250" terminal)                       ^
     |                                  |
 High-temp                         10kΩ  |
    wire                               |
     |                                 |
   100kΩ                               |
     |                                 |
     +----[BAT54 Cathode]              |
     |         |                       |
        |      [Anode]                    |
     |         |                       |
     |        GND                      |
     |                                 |
     +----------+                      |
                |              +-------+
                |              |
                |              |    +3.3V
                |              |      ^
                |              |      |
                v              v      |
      LMP7721  [8]           [4]     [6]
      +-------+----+---------+--------+
      |  IN-  | VOUT|        |  V+    |
      |       |     |        |        |
      | [1]   | [3] | [2][5] | [7]    |
      +---+---+--+--+---+----+---+----+
          |   |  |      |        |
        IN+   | V-    N/C     Guard
          |   |  |      |        |
         GND  | GND   Guard    Guard
              |       Ring     Ring
              |        |        |
             GND      GND      GND

GROUND ROD (welded to mounting bracket) → Burner chassis → Star ground

Feedback Network (between VOUT and IN-):
- Rf: 220kΩ + 100kΩ in series = 320kΩ
- Cf: 10pF in parallel with Rf

Output Filter (at ADC Node):
- 0.1µF capacitor to GND

Power Supply Decoupling:
- 0.1µF + 10µF near V+ pin to GND
```

## Compact ASCII Wiring Diagram (quick reference)

```
 Flame Rod (electrode)
      |
   High-temp lead
      |
   100kΩ (Rin)
      |-------------------------------+--------------------- to GND via BAT85 (cathode at node)
      |                               |
      |                          ┌────┴────┐
      +--------------------------│  LMP7721 │
                                              │          │
 IN+ (pin1) ──── GND      V- (pin3) ── GND
 IN- (pin8) ◄─────┐       V+ (pin6) ── 3V3
                            │
                Rf ≈320kΩ (220k + 100k series)
                            │
 OUT (pin4) ───────┘
      |
    10kΩ (Rout)
      |
   ADC node ──────────> ESP32‑C6 A1 (GPIO1/ADC1_CH1)
      |
   0.1µF (Cout) to GND

 Decoupling at LMP7721 V+:
   - 0.1µF ceramic to GND
   - 10µF electrolytic to GND

 Feedback capacitor Cf:
   - 10 pF (C0G) in parallel with Rf (OUT↔IN−)
```

## Step-by-Step Assembly Instructions

### Step 1: Power Supply Connections
```
Red wire (Pin 6, V+) → 3.3V supply
Green wire (Pin 3, V-) → GND  
Green wire (Pin 1, IN+) → GND
```

### Step 2: Feedback Resistor Network
You need **320kΩ total** for Rf. From your parts:
- Use **220kΩ + 100kΩ in series** (from your dev plan parts list)
- Connect between Blue wire (Pin 4, VOUT) and Blue wire (Pin 8, IN-)

### Step 3: Feedback Capacitor (10pF)
- Connect **10pF capacitor** in parallel with the 320kΩ feedback resistor
- One terminal to Blue wire (Pin 4, VOUT)  
- Other terminal to Blue wire (Pin 8, IN-)

### Step 4: Flame Rod Connection (Based on Your S-Probe Drawing)
From your technical drawing:
- **ELECTRODE ROD** (insulated, center rod) → 0.250" terminal → High-temp wire → **100kΩ resistor** → Blue wire (Pin 8, IN-)
- **GROUND ROD** (outer rod, welded to bracket) → Mounting bracket → Burner chassis → Star ground

### Step 5: Input Protection (Single BAT54)
Since you only have **1 BAT54 diode**, use it for the most critical protection:
- **BAT54**: Cathode to input junction (after 100kΩ), Anode to GND
- This protects against negative voltage spikes during ignition

Note: With only one diode, you get protection against negative transients. The LMP7721's internal protection will handle positive overload.

### Step 6: Output Filter
- **10kΩ resistor** from Blue wire (Pin 4, VOUT) to ADC node
- **0.1µF capacitor** from ADC node to GND
- **ADC node** → ESP32-C6 A1 pin

### Step 7: Power Supply Decoupling
Near the LMP7721:
- **0.1µF capacitor** between Pin 6 (V+) and Pin 3 (V-)
- **10µF capacitor** between Pin 6 (V+) and Pin 3 (V-)

### Step 8: Guard Ring (Optional but Recommended)
- Connect Pin 2, Pin 5, and Pin 7 to a copper guard ring
- Connect guard ring to GND

## Physical Layout Tips

### Component Placement Priority:
1. **Keep feedback components close**: 320kΩ resistor and 10pF cap right at the op-amp
2. **Short high-impedance traces**: Minimize length of IN- connections
3. **Star ground**: All grounds meet at one point
4. **Separate analog/digital grounds**: Keep switching currents away from TIA ground

### Wire Management:
- **Blue wires**: Both go to high-impedance nodes - keep them short and away from switching signals
- **Green wires**: Both are ground connections - can be longer
- **Red wire**: Power connection - add decoupling capacitors close to pin

## Testing Procedures

### Bench Test (No Flame):
1. **Power Check**: Verify 3.3V at Pin 6, 0V at Pin 3
2. **Bias Check**: Pin 1 should be ~0V, Pin 8 should be ~0V (virtual ground)
3. **Dummy Current Test**:
   ```
   3.3V → 3.3MΩ resistor → Input junction (after 100kΩ)
   Expected output: ~1µA × 320kΩ = 0.32V at VOUT
   ```

### Noise Test:
- Toggle any switching loads (glow plug, relays)
- VOUT should remain stable (clamps and filter working)

### ADC Calibration:
- Inject known currents: 1µA, 5µA, 10µA
- Record ADC readings
- Create lookup table for firmware

## Component Values Summary (From Your Parts)

| Component | Value | Purpose | Your Part |
|-----------|--------|---------|-----------|
| Rf | 320kΩ | Transimpedance gain | 220kΩ + 100kΩ series |
| Cf | 10pF | Stability & noise | 10pF ceramic |
| Rin | 100kΩ | Input series protection | 100kΩ metal film |
| Rout | 10kΩ | Output filter | 10kΩ metal film |
| Cout | 0.1µF | Output filter | 0.1µF ceramic |
| Cbypass | 0.1µF + 10µF | Power decoupling | Ceramic + electrolytic |
| D1, D2 | BAT54 | Input protection | Schottky diodes |

## Expected Performance

### Signal Chain:
```
0.5µA flame current → 0.16V at VOUT → ~0.13V at ADC (after filter)
10µA flame current → 3.2V at VOUT → ~2.6V at ADC (after filter)
ADC range: 0-3.3V → Full flame detection range covered
```

### Frequency Response:
- TIA bandwidth: ~1.6MHz (limited by Cf)
- Output filter: ~1.6kHz cutoff (rejects ignition noise)
- Sample rate: 100-200Hz (adequate for flame detection)

## Troubleshooting Guide

### VOUT stuck at 0V:
- Check power supply (3.3V at Pin 6)
- Verify feedback resistor connections
- Test with dummy current injection

### VOUT saturated high (>3.1V):
- Check for leakage at input (clean with IPA)  
- Verify protection diodes are connected correctly
- Ensure IN+ is grounded

### Noisy/oscillating output:
- Add/check 10pF feedback capacitor
- Verify ground connections (star ground)
- Check power supply decoupling

### Low sensitivity:
- Verify 320kΩ feedback resistance
- Check for short circuits at high-impedance nodes
- Clean flux residue around input connections

This circuit will convert your flame rod's 0.5-10µA rectification current into a clean 0.16-3.2V signal for the ESP32-C6 ADC.
