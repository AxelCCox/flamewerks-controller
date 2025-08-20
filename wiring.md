# FlameWerks Single-Burner Wiring Diagram (ESP32-C6 Thing Plus)

This document maps the electrical wiring for one burner using SparkFun ESP32-C6 Thing Plus and components listed in your dev plan. It reflects the pin choices used in `ESP32-C6-Firmware/FlameWerks_ESP32C6.cpp` and the safety requirements.

Legend:
- HV = High Voltage 12 V domain (valves, glow)
- LV = Low Voltage 3.3 V domain (ESP32-C6, sensors)
- NC = Normally Closed

---

## Overview

12 V input (fused) powers valves and glow plug. ESP32-C6 runs at 3.3 V from the board’s onboard regulator. All grounds meet at a star ground near power entry. Flame sensing uses a transimpedance amplifier (TIA) with LMP7721, clamped and filtered into an ESP32-C6 ADC pin.

---

## SparkFun ESP32-C6 Thing Plus onboard connections (jumpers)

Per the board docs, some GPIO are pre-wired to on-board peripherals through solder jumpers:

- IO18 → CS (SPI chip-select)
- IO22 → microSD Detect
- IO11 → MAX17038 Fuel Gauge Alert
- IO23 → WS2812 Status LED (data in)
- IO15 → Low Power control

Recommendation: Avoid these pins for external wiring unless you cut/reconfigure the jumpers. The wiring map below intentionally uses other GPIO to prevent conflicts.

## Pin/Connection Map

Controller: SparkFun ESP32-C6 Thing Plus

- GPIO2  -> Pilot Valve driver input (OUT1, active HIGH)
- GPIO4  -> Main Valve driver input (OUT2, active HIGH)
- GPIO5  -> Glow Plug MOSFET Gate (OUT3, active HIGH)
- GPIO9  -> Emergency Stop input (NC to GND, use INPUT_PULLUP)
 - ADC input: A1 header (ADC1_CH1) -> Flame Sense analog input (TIA output)
  - Assumes A1 maps to GPIO1/ADC1_CH1 on the Thing Plus. Please confir m against the SparkFun pinout; if different, set firmware to the GPIO that corresponds to A1.
  - Reason: Avoids IO0 boot-strap sensitivity; A1 is a cleaner choice for the TIA.
- 3V3    -> LV supply for logic and TIA
- GND    -> Logic ground; join to star ground
- Qwiic (IO6=SDA, IO7=SCL) -> Reserved for I2C expansion (future)

Status LED (optional): GPIO8 -> LED (with series resistor) to 3V3 or GND depending on LED wiring.
- Note: The board already has a WS2812 status LED on IO23 via jumper. To use it from firmware, target IO23; otherwise keep a discrete LED on IO8 to avoid jumper changes.

---

## Power Distribution

- 12 V DC IN -> Main fuse (e.g., 5–10 A depending on glow + valves) -> Star power node
  - Branch A: 12 V -> Valve coils (through drivers) -> flyback diodes to 12 V/GND as appropriate
  - Branch B: 12 V -> Glow plug -> N-MOSFET -> Shunt (optional) -> GND; separate 10–20 A fuse sized for glow
  - Branch C: 12 V -> ESP32-C6 Thing Plus VIN (onboard 3.3 V regulator)

- Grounds: Run separate returns from coils and glow to star ground. Keep TIA/sensor analog ground isolated from high current paths; tie at star point.

---

## Valve Drivers (12 V solenoids)

Option 1: Relay Module (opto-isolated)
- ESP GPIO2/4 -> Relay IN1/IN2 via opto inputs (observe module logic level)
- Relay COM -> 12 V; Relay NO -> Valve +; Valve - -> GND
- Flyback: If using bare relay coil drivers, diode across coil. If using solid-state MOSFET driver, flyback across valve coil.

Option 2: N-MOSFET Low-side switching
- ESP GPIO2/4 -> Gate via 100 Ω resistor; 10 kΩ gate pulldown to GND
- MOSFET Source -> GND; Drain -> Valve -; Valve + -> 12 V
- Flyback diode (e.g., SS14/FR107) across valve coil (anode to GND, cathode to 12 V)
- Choose logic-level MOSFET with low Rds(on) at 3.3 V gate (e.g., AOZ, IRLZ series).

---

## Glow Plug Driver (12 V, high current)

- ESP GPIO5 -> Gate via 100 Ω; 10 kΩ gate pulldown to GND
- N-MOSFET Source -> GND; Drain -> Glow Plug -; Glow Plug + -> 12 V (dedicated fuse)
- Add TVS diode (e.g., SMBJ15A) across 12 V rail near glow to clamp transients
- Heatsink MOSFET appropriately; size wiring for inrush/steady current

---

## Emergency Stop Input (Safety NC)

- Hardware E-Stop switch (NC contact): One side to ESP GPIO9, other side to GND
- Configure GPIO9 as INPUT_PULLUP; logic HIGH means open circuit (E-Stop pressed)
- Optional: RC debounce 10 kΩ series + 100 nF to GND at pin

---

## Flame Sensing Front-End (TIA with LMP7721)

Goal: Convert flame rectification current (~0.5–10 µA) into a voltage for ADC.

Schematic blocks:
- Flame Rod -> series R 100 kΩ -> TIA inverting input (op-amp -)
- LMP7721 op-amp
  - Feedback Rf ≈ 320 kΩ (220k + 100k in series) between OUT and -
  - Feedback Cf = 10 pF in parallel with Rf
  - Non-inverting input (+) to GND (single-supply, positive current -> positive Vout)
  - Supply: 3.3 V to V+; GND to V-
- Input clamps: BAT54 diodes from TIA input node to GND and to OUT (or to a small Vref) to survive ignition transients
- Post-filter: 10 kΩ series from OUT -> node -> 0.1 µF to GND; node -> ADC1_CH0 (IO0)
- Optional bulk decoupling: 0.1 µF + 10 µF near op-amp V+

Expected scaling: 10 µA -> ~3.2 V; 1 µA -> ~0.32 V; sample ADC at 100–200 Hz and average.

Layout tips:
- Keep TIA input path short and clean; guard ring around - input if possible
- Keep high current switching (valves/glow) away from TIA traces
- Star ground; single tie-in of TIA ground to star

---

## Textual Wiring Diagram

12 V IN (+) -- Main Fuse --+---> Valve PWR (+)
                            |\
                            | +-> Glow Plug (+) [Glow Fuse]
                            | +-> ESP32-C6 VIN
                            +---> TVS to GND near glow

Valve Pilot (-) ---+-> MOSFET/Relay OUT1 -> GND
                   +-> Flyback diode across coil (cathode to +12 V)

Valve Main (-) ----+-> MOSFET/Relay OUT2 -> GND
                   +-> Flyback diode across coil (cathode to +12 V)

Glow Plug (-) -----> Power MOSFET Drain
MOSFET Source ------> GND (star)
Gate (GPIO5) --100 Ω--> | Gate; 10 kΩ to GND

E-Stop NC -------- GPIO9
E-Stop other ----- GND
(GPIO9 uses INPUT_PULLUP)

Flame Rod ----100 kΩ----> TIA (-)
TIA (+) --------------- GND
Rf 320 kΩ // Cf 10 pF between OUT and -
BAT54 clamps at input node (to GND and OUT)
OUT --10 kΩ--> ADC node --0.1 µF--> GND -> A1 (ADC1_CH1)

ESP32-C6 3V3 -> Op-amp V+
GND (star) -> Op-amp V-

---

## Bill of Materials (core)

- Controller: SparkFun ESP32-C6 Thing Plus (DEV-22924)
- Valves: 12 V solenoids (Pilot, Main)
- Igniter: 12 V glow plug
- MOSFETs: Logic-level N-FETs for valves (optional) and required for glow (>=30 A peak)
- Diodes: BAT54 (clamps), Flyback diodes (SS14/FR107), TVS (SMBJ15A)
- Op-amp: TI LMP7721MA
- R/C: 220k + 100k (Rf), 10 pF (Cf), 100 kΩ (series rod), 10 kΩ (post filter), 0.1 µF, 10 µF
- E-Stop: NC latching switch
- Fuses: Main inline; separate fuse for glow
- Enclosure: NEMA-rated with cable glands

---

## Notes

- If using relay modules, verify input logic (active LOW vs HIGH) and adjust firmware pin polarity.
- Confirm ESP32-C6 Thing Plus ADC-capable pin mapping; if IO0 is reserved/boot-strapped on your board, select another ADC1 channel and update firmware macro.
- Keep wiring from flame rod well-shielded and away from igniter wiring.
- Validate E-Stop latency at outputs (<100 ms).
