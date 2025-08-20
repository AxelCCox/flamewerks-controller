# Flame Sensing Rods + Transimpedance Amplifier (TIA) — Build Sheet

Goal: Wire and mount your two-prong flame rod assembly to a low-leakage TIA using TI LMP7721, producing a clean ADC signal (A1 / ADC1_CH1) for the ESP32‑C6.

This guide is specific to:
- Sensor: two-rod assembly (one or both rods insulated from the metal bracket) — image provided
- Op‑amp: TI LMP7721MA in SOIC‑8
- Controller: SparkFun ESP32‑C6 Thing Plus (ADC A1)

---

## 1) LMP7721 SOIC‑8 Pin Map (from datasheet)

Top view (pins when looking down on the chip):
- Pin 1 — IN+ (non‑inverting)
- Pin 2 — N/C (recommend connect to guard)
- Pin 3 — V− (GND)
- Pin 4 — VOUT (output)
- Pin 5 — N/C (recommend connect to guard)
- Pin 6 — V+ (3.3 V)
- Pin 7 — N/C (recommend connect to guard)
- Pin 8 — IN− (inverting)

Guard recommendation: Route pins 2/5/7 to a local “guard ring” copper tied to 0 V (same potential as IN+ for this design). Keep this guard surrounding the IN− pin and input trace to reduce leakage.

---

## 2) Electrical Schematic (values from your parts)

- Flame rod lead → 100 kΩ series resistor → IN− (Pin 8)
- Feedback: Rf ≈ 320 kΩ (220 kΩ + 100 kΩ in series) from VOUT (Pin 4) to IN− (Pin 8)
- Feedback capacitor: Cf = 10 pF in parallel with Rf (from VOUT to IN−)
- IN+ (Pin 1) → GND (0 V)
- V+ (Pin 6) → 3.3 V
- V− (Pin 3) → GND (0 V)
- Output post‑filter (to tame ignition noise):
  - VOUT (Pin 4) → 10 kΩ → ADC node
  - ADC node → 0.1 µF → GND (forms ~1.6 kHz RC)
  - ADC node → ESP32‑C6 A1 (ADC1_CH1)
- Protection at input node (junction of 100 kΩ and IN−):
  - BAT54 Schottky to GND (anode at GND, cathode at input node)
  - BAT54 Schottky to VOUT (anode at input node, cathode at VOUT)
- Decoupling: 0.1 µF and 10 µF close to Pin 6 (3.3 V) to Pin 3 (GND)

Notes
- The virtual ground at IN− sits near 0 V (because IN+ is at 0 V). Positive flame current into IN− produces positive VOUT ≈ I_flame × Rf.
- With Rf ≈ 320 kΩ, 0.5–10 µA → ~0.16–3.2 V at VOUT (before RC).

---

## 3) Using Your Two‑Rod Assembly

Identify the rods:
- Many assemblies have both rods insulated from the bracket (white ceramic). The bracket is metal and usually bolted to the burner—this is chassis ground.
- We will use a single rod as the sensor (isolated). The return path is the grounded burner metal through the flame.

If both rods are insulated:
- Choose ONE rod as the sensor. Connect its lead (high‑temp wire) to the 100 kΩ series resistor → IN−.
- Optionally tie the second rod to the bracket (ground) to increase reference area: add a ring lug or jumper from the second rod’s terminal to the bracket ground screw. If you do this, ensure good metal‑to‑metal contact.

If only one rod is insulated and the other is bonded to bracket:
- Use the insulated rod as the sensor. The bonded rod/bracket acts as ground reference automatically.

Placement guidelines (pilot flame region):
- Tip of the sensing rod should sit in the stable pilot flame envelope, not just the ignition plume.
- Target 3–8 mm immersion in flame; avoid direct contact with the burner metal to prevent shorting.
- Maintain at least 3 mm creepage to grounded metal along the ceramic.
- Keep any glow plug or HV igniter leads physically separated from the sensor lead.

---

## 4) Wiring, Cable, and Grounding

- Sensor lead: Use high‑temp, insulated wire from the rod to the electronics enclosure. Keep this run as short as practical.
- Shielding: Use a shielded cable if the run exceeds ~30 cm or if ignition noise is high. Connect shield to chassis/earth at the enclosure end only (single‑point). Do NOT connect shield to the op‑amp input node.
- Enclosure entry: Use a gland. Keep the TIA PCB as close as possible to the entry to minimize the exposed high‑impedance trace length.
- Star ground: Tie the TIA ground to the star point where all returns meet. Do not share the same return trace as glow/valve currents.

PCB/Perfboard layout tips:
- Place the LMP7721, Rf, Cf, and the input node within a tight cluster.
- Surround the IN− net with a grounded guard ring tied to 0 V. Connect pins 2/5/7 to this guard copper.
- Keep the 100 kΩ input resistor right at the op‑amp input pad. Avoid flux residue around the input; clean with IPA.
- Route VOUT away from the input node; keep the RC filter tight to the op‑amp.

---

## 5) Assembly Steps (checklist)

1) Mount the rod bracket to the burner so the sensing rod sits in a steady pilot flame.
2) Choose the sensing rod. Run its high‑temp lead to the enclosure through a gland.
3) Build the TIA on a small board:
   - LMP7721 + decoupling
   - 100 kΩ series at IN−
   - Rf 220k + 100k in series from VOUT to IN−
   - Cf 10 pF in parallel with Rf
   - BAT54 clamps at the IN− node to GND and to VOUT
   - RC post‑filter (10 kΩ/0.1 µF) at VOUT → ADC node
4) Power the op‑amp from 3.3 V. Verify V+≈3.3 V, V−=0 V.
5) Connect ADC node to ESP32‑C6 A1 (ADC1_CH1). Common ground back to star point.
6) Before gas, bench‑test (Section 6). Then integrate on burner and test with flame.

---

## 6) Bench Test & Calibration

Dummy current injection:
- Create ~1 µA source: 3.3 V → 3.3 MΩ → into the IN− node (after the 100 kΩ), then to op‑amp. Alternatively, 5 V → 4.7 MΩ ≈ 1.06 µA.
- Expect VOUT ≈ I × Rf ≈ 1 µA × 320 kΩ ≈ 0.32 V (minus small offsets).
- Verify ADC reading at A1 increases correspondingly.

Noise check:
- Toggle glow and valve drivers while monitoring VOUT. The post‑filter should suppress large spikes; the clamps protect the input.
- If noise persists, increase Cf slightly (e.g., 15–22 pF) and/or increase the post‑filter C (0.22 µF) while keeping response time suitable.

Firmware sanity:
- Sample at 100–200 Hz; moving average (8–16 samples); apply hysteresis threshold.
- Log raw ADC and computed µA during tests to set thresholds.

---

## 7) Safety & Reliability

- Keep sensor wiring away from hot metal; use ceramic stand‑offs if needed.
- Use high‑temp sleeving on the rod lead near the flame zone.
- Confirm E‑Stop cuts power to valves and glow; ensure the TIA/ADC reading is non‑controlling during E‑Stop.
- Add a small series resistor (e.g., 1 kΩ) right at the op‑amp output if the cable to MCU is long to reduce ringing.

---

## 8) Quick Connection Table

- Rod (sensor) → 100 kΩ → LMP7721 Pin 8 (IN−)
- LMP7721 Pin 1 (IN+) → GND
- LMP7721 Pin 3 (V−) → GND
- LMP7721 Pin 6 (V+) → 3.3 V
- LMP7721 Pin 4 (VOUT) → 10 kΩ → ADC node → ESP32‑C6 A1; ADC node → 0.1 µF → GND
- BAT54: input node ↔ GND; input node ↔ VOUT (orientation as in Section 2)
- Rf: 220 kΩ + 100 kΩ series between VOUT and IN−
- Cf: 10 pF in parallel with Rf
- Pins 2/5/7 → guard copper tied to GND (local guard ring)

---

## 9) Troubleshooting

- VOUT stuck near 0 V:
  - No current path (rod not in flame); check connections; try dummy µA injection.
- VOUT saturates high (>3.1 V) with no flame:
  - Leakage or noise coupling into input; clean board; add shielding; verify clamps; check that second rod/bracket is grounded.
- Large spikes when glow turns on:
  - Verify TVS at 12 V rail; increase Cf slightly; move TIA ground to star point; add shielding.

---

If you want, I can turn this into a KiCad schematic/board outline next, or add a small printable one‑page wiring card for installers.

---

## 10) Map to Your S‑Probe Drawing (Electrode + Ground Rod)

Based on the drawing you shared (two parallel rods with a glazed insulator, a mounting bracket, and a 0.250" faston terminal):

- Electrode rod (the one passing through the glazed insulator) = SENSE ROD
  - Connection: The terminal/tab that belongs to the insulated electrode goes via high‑temp wire → 100 kΩ series → LMP7721 Pin 8 (IN−).
  - Keep this lead short where it enters the enclosure; use shielded cable if long. Shield to chassis/earth at the enclosure end only.

- Ground rod (parallel rod bonded/welded to the bracket) = RETURN/GROUND REFERENCE
  - Connection: Ensure the mounting bracket has a solid, low‑resistance bond to burner chassis ground (star to the power ground hub).
  - Optionally add a braided ground strap or star washer to guarantee metal‑to‑metal contact.

- Spark gap callout in the drawing
  - If you are NOT using direct‑spark ignition (you use a glow plug), leave any HV ignition lead unconnected. Do NOT tie any HV source into the TIA/sense rod.
  - If you later adopt DSI, the same insulated electrode can be used for spark, but it must be switched and isolated from the TIA during ignition (use an HV coupling network and a protection switch; not part of this TIA build).

- Terminal 0.250" × 0.032" (faston spade)
  - Use a high‑temp female quick‑disconnect crimp rated for the environment; add fiberglass sleeving where it exits the flame zone.

Placement, per the drawing dimensions
- Position the electrode tip within the stable pilot flame envelope. Typical immersion 3–8 mm; keep ~3 mm creepage along the ceramic to any grounded metal.
- Maintain the specified spark gap if you plan to use DSI; for pure sensing, the exact gap is less critical, but the ground rod still provides a good reference surface.

Final connections recap (as they relate to the drawing labels)
- Electrode rod terminal → high‑temp wire → 100 kΩ → LMP7721 Pin 8 (IN−)
- LMP7721 Pin 4 (VOUT) ↔ Rf (220k + 100k) ↔ Pin 8 (IN−) and Cf 10 pF in parallel
- Input protection at the IN− node: BAT54 to GND and BAT54 to VOUT (orientations per Section 2)
- VOUT → 10 kΩ → ADC node → ESP32 A1 (ADC1_CH1); ADC node → 0.1 µF → GND
- Ground rod + mounting bracket → burner chassis → star ground (shared 0 V)
