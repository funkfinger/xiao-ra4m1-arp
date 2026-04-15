# Generative Arpeggiator — Eurorack CV/Gate Module

> **Rev 0.1 — April 2026**
> 4 HP · XIAO RA4M1 · 4 Jacks · 3 Pots · WS2812B RGB · Reversible Panel

---

## Related Notes

- [[MOD2 Platform]]
- [[Melon Module (WGD Modular)]]
- [[XIAO RA4M1]]
- [[Eurorack V/Oct Scaling]]

---

## 1. Project Overview

A 4 HP Eurorack generative arpeggiator module producing scale-quantised V/Oct pitch CV and gate outputs, driven by a probability-layered generative engine inspired by the Ableton arpeggiator / random MIDI-effect workflow.

The design is based on Hagiwo's MOD2 platform and WGD Modular's Melon rework, extending both with a real 12-bit DAC output for accurate V/Oct, a WS2812B RGB LED for scale/mode feedback, and a fully symmetric PCB layout enabling a reversible front panel.

### 1.1 Design Goals

- True 12-bit V/Oct pitch CV via RA4M1 onboard DAC — no external DAC chip required
- Scale-quantised generative melody with independently controllable pitch variance, octave probability, and gate density
- Reversible / symmetrical 4 HP front panel — module can be mounted either orientation
- WS2812B RGB LED communicates current scale/mode at a glance
- Melon-compatible form factor: same XIAO socket, jack/pot footprints, 2x5 power header
- Fully open-source hardware (KiCad) and firmware (Arduino/C++)

### 1.2 Relationship to Existing Designs

| Platform | What We Inherit | What We Change |
|---|---|---|
| Hagiwo MOD2 | Circuit topology, protection diodes, PWM output stage, pot/jack layout | Replace PWM CV with real DAC; add op-amp scaling; add WS2812B |
| WGD Melon | Symmetric PCB layout, onboard 5V reg, 2x5 power header, WS2812B pad | Replace RP2350 with RA4M1; customise I/O for arp use-case |
| This module | All of the above | RA4M1 DAC → op-amp V/Oct output; dedicated arp firmware; reversible panel silkscreen |

---

## 2. Hardware Specification

### 2.1 Module Format

| Parameter | Value |
|---|---|
| Format | Eurorack 3U |
| Width | 4 HP (20.32 mm) |
| Depth | ~35 mm (including power header) |
| Power connector | 2x5 shrouded Eurorack header |
| Power rails used | +12V, -12V (brought to PCB, op-amp is single-supply), +5V via onboard AMS1117-5.0 |
| Power draw (est.) | +12V: ~60 mA · -12V: ~10 mA · 5V bus: 0 mA (self-generated) |

### 2.2 MCU — Seeed XIAO RA4M1

The Renesas RA4M1 was chosen over the RP2350 specifically for its onboard true 12-bit DAC, eliminating the need for an external I2C DAC chip and all associated complexity.

| Attribute | Value |
|---|---|
| MCU | Renesas RA4M1, ARM Cortex-M4 @ 48 MHz with FPU |
| Flash | 256 KB |
| SRAM | 32 KB |
| ADC | 14-bit, up to 12 channels |
| DAC | 12-bit, 2 channels (DAC0 used for V/Oct) |
| GPIO | 19 total (including 8 back-side expansion pads) |
| Interfaces | I2C, SPI, UART, USB 2.0, CAN Bus |
| Form factor | XIAO standard (21 x 17.8 mm), SMD stamp holes |
| Arduino IDE | Fully supported |
| Price (approx.) | ~$5 USD |

### 2.3 Pin Assignment

| XIAO Pin | RA4M1 GPIO | Direction | Function | Notes |
|---|---|---|---|---|
| D0 / A0 | P014 | Out | DAC0 → V/Oct CV | True 12-bit DAC; feeds op-amp scaling stage |
| D1 / A1 | P000 | In | CV In (J2) | 14-bit ADC; 0–5V → 0–3.3V via divider |
| D2 / A2 | P001 | In | Pot RV1 — Scale select | 14-bit ADC; wiper 0–3.3V |
| D3 / A3 | P002 | In | Pot RV2 — Chaos amount | 14-bit ADC; wiper 0–3.3V |
| D6 | P301 | Out | Gate Out (J4) | 3.3V logic → 5V via NPN transistor |
| D7 | P303 | In | Clock / Gate In (J1) | Protected digital input |
| D8 / A8 | P304 | In | Pot RV3 — Rate/Tempo | 14-bit ADC; wiper 0–3.3V |
| D9 | P203 | Out | WS2812B RGB data | 330 Ω series resistor |
| D10 | P205 | In | Tact switch SW1 | INPUT_PULLUP in firmware |
| D4 | P101 | — | I2C SDA (reserved) | Available for future expansion / orientation sensing |
| D5 | P102 | — | I2C SCL (reserved) | Available for future expansion |

### 2.4 Input Circuits

#### 2.4.1 Gate / Clock Input (J1) — Digital

Eurorack gate signals are 0–5V or 0–10V unipolar. RA4M1 GPIO tolerates 3.3V max. Protection:

- 3.3 kΩ series resistor (current limiting)
- Schottky diode clamp to 3.3V rail (BAT54 or equivalent)
- Schottky diode clamp to GND
- Threshold set in firmware: >1.5V = HIGH

> **Note:** Same protection topology as MOD2. Tested to survive ±12V continuous per HAGIWO's durability test protocol.

#### 2.4.2 CV Input (J2) — Analog

CV input shares an ADC channel with Pot RV3 following the MOD2 convention. When a jack is inserted, the CV signal overrides the pot. Input range is 0–5V Eurorack, scaled to 0–2.5V for the ADC via a 100 kΩ / 100 kΩ resistor divider. Protection diodes as per J1.

> **Note:** The op-amp output stage inverts the signal. CV HIGH = lower ADC reading. Firmware compensates.

### 2.5 V/Oct Output Stage

The RA4M1 DAC0 pin outputs 0–3.3V (12-bit = 4096 steps). This is scaled and buffered to Eurorack V/Oct standard using an op-amp stage.

#### 2.5.1 Target Output Range

Unipolar 0–4.0V output covering ~4 octaves (C2–C6), adequate for a melodic arpeggiator. Bipolar ±5V is a stretch goal (see Section 7).

#### 2.5.2 Op-Amp Scaling Circuit

Op-amp: **MCP6002** (single-supply, rail-to-rail, 5V powered). Non-inverting amplifier with offset trim.

| Component | Value | Function |
|---|---|---|
| U2 | MCP6002T-I/SN SOIC-8 | Dual op-amp; single-supply; rail-to-rail; 5V powered |
| R1 | 10 kΩ | Input resistor |
| R2 | 12.1 kΩ | Gain resistor — sets gain ≈ 2.21x to map 0–3.3V → 0–4.0V |
| R3 | 100 kΩ trim pot | Fine offset adjustment for V/Oct calibration |
| R4 | 1 kΩ | Output series resistor (standard Eurorack output impedance) |
| C1 | 100 nF | Op-amp supply decoupling |
| C2 | 10 nF | Op-amp output stability / HF rolloff |

> **Note:** Gain formula: `Vout = Vdac × (1 + R2/R1)`. With R1=10k, R2=12.1k: gain = 2.21. Adjust R3 trim for zero-offset at power-up.

### 2.6 Gate Output Stage (J4)

RA4M1 outputs 3.3V logic. Eurorack gate convention is +5V. NPN transistor level-shift:

- Base: D6 via 1 kΩ resistor
- Collector: +5V rail via 10 kΩ pull-up
- Emitter: GND
- Output taken from collector; 1 kΩ series output resistor to jack

> **Note:** Gate high ≈ 4.9V. Rise/fall time < 1 µs. No smoothing cap on this output (unlike MOD2's CV outs) to avoid gate delay.

### 2.7 RGB LED — WS2812B

| Item | Value / Note |
|---|---|
| LED | WS2812B PLCC4 5.0x5.0mm |
| Data resistor | 330 Ω series on data line |
| Decoupling | 100 nF across LED VCC/GND, placed as close as possible |
| Power | 5V rail; typical ~60 mA at full white (colour use is much lower) |
| Library | Adafruit NeoPixel (Arduino) |
| Footprint note | PCB has pads for both WS2812B and a 3mm through-hole LED — populate one only |

### 2.8 Potentiometers

| Ref | Value | Taper | Function | Firmware Range |
|---|---|---|---|---|
| RV1 | 100 kΩ | Linear | Scale / Mode select | Quantised to 6 scale types |
| RV2 | 100 kΩ | Linear | Chaos / Randomness | 0% = deterministic → 100% = full random within scale |
| RV3 | 100 kΩ | Linear | Rate / Tempo | ~20 BPM to ~300 BPM internal clock |

> **Note:** Footprint: Alpha RD901F-40-00D 9mm vertical PCB-mount. Same as Melon / MOD2.

### 2.9 Power Supply

| Component | Value | Function |
|---|---|---|
| J5 | 2x5 shrouded 2.54mm | Eurorack bus connection |
| D1 | 1N5819 Schottky SOD-123 | Reverse polarity protection on +12V |
| U1 | AMS1117-5.0 SOT-223 | +12V → +5V LDO regulator |
| C3 | 100 µF electrolytic | Input bulk cap for AMS1117 |
| C4 | 10 µF electrolytic | Output bulk cap for AMS1117 |
| C5, C6 | 100 nF ceramic | HF decoupling on +12V and +5V rails |
| C7 | 100 µF electrolytic | Bulk decoupling on -12V rail |
| C8 | 100 nF ceramic | HF decoupling on -12V rail |

> **Note:** -12V rail is brought to PCB and decoupled as best practice but no Rev 1 components draw from it. Required for the bipolar V/Oct stretch goal.

---

## 3. Panel & PCB Design

### 3.1 Component Layout (Symmetric)

The panel and PCB are designed with 180° rotational symmetry around the module centre point. Every hole maps to another hole of the same type when the panel is flipped.

| Position | Component | Type | Signal |
|---|---|---|---|
| Top | J1 | 3.5mm jack | Clock / Gate In |
| Upper-mid | J2 | 3.5mm jack | CV In |
| Centre-top | RV1 | 9mm pot | Scale select |
| Centre | RV2 | 9mm pot | Chaos amount |
| Centre-bot | RV3 | 9mm pot | Rate / Tempo |
| Lower-mid | J3 | 3.5mm jack | V/Oct CV Out |
| Bottom | J4 | 3.5mm jack | Gate Out |
| Centre (PCB) | D13 | WS2812B | Scale / mode colour |
| Centre (PCB) | SW1 | 6mm tact | Reset / mode cycle |

> **Note:** LED and tact switch are on the PCB face behind the panel. LED shines through a 3mm drill in the panel; switch actuator protrudes through a matching hole.

### 3.2 Reversibility

The symmetric layout means the panel can be removed, flipped 180°, and reinstalled with all holes aligning correctly. This enables:

- Left-hand vs right-hand placement flexibility within a rack
- A "dark side / light side" aesthetic if two panel finishes are used (e.g. matte black one side, brushed aluminium the other)
- Physical identification of module orientation in dense racks

Panel text/silkscreen must work in both orientations:

- **Symbol-only labelling** — arrows for in/out, waveform icons for function, no text
- Or rotational-safe abbreviations printed at both ends of each hole (0° and 180°)

> **Stretch Goal — see Section 7**

### 3.3 PCB Fabrication Notes

- 2-layer FR4, 1.6 mm thickness
- Minimum trace width: 0.2 mm signal, 0.5 mm power
- JLCPCB-compatible: SMT parts from JLCPCB Basic Parts library where possible
- 100 x 100 mm or smaller to stay within lowest-cost JLCPCB tier
- Gerbers + BOM + CPL generated from KiCad for JLCPCB PCBA service
- Through-hole parts (jacks, pots, LED, switch, power header) hand-soldered post-reflow

---

## 4. Firmware Architecture

### 4.1 Overview

Arduino C++ targeting the Seeed XIAO RA4M1, using the Renesas Arduino core. Cooperative loop with timer-driven clock — no RTOS required.

### 4.2 Generative Engine

Four independently controllable probability layers:

| Parameter | Control | Behaviour |
|---|---|---|
| Scale / Mode | RV1 + CV in J2 | Selects from 6 scales. Quantises all random output to chosen scale. |
| Chaos (Pitch) | RV2 | Probability (0–100%) that the current step wanders ±N scale degrees from the base arp note. |
| Octave Jump | Derived from RV2 | Independent sub-probability (~RV2/3) of jumping +1 octave. Adds brightness without full chaos. |
| Gate Density | Derived from RV2 | Probability that a gate fires at all. Higher chaos = more rests, rhythmic irregularity. |
| Rate | RV3 | Internal clock BPM (20–300). Overridden by external clock on J1 if patched. |

### 4.3 Scale Quantiser

All pitch output is quantised to the selected scale before DAC write. This is the key element that keeps random output musical regardless of chaos amount.

| Scale | Intervals (semitones from root) | LED Colour |
|---|---|---|
| Major | 0, 2, 4, 5, 7, 9, 11 | Warm white |
| Natural Minor | 0, 2, 3, 5, 7, 8, 10 | Deep blue |
| Pentatonic Major | 0, 2, 4, 7, 9 | Bright green |
| Pentatonic Minor | 0, 3, 5, 7, 10 | Purple |
| Dorian | 0, 2, 3, 5, 7, 9, 10 | Cyan |
| Chromatic | 0–11 | White (cycling hue) |

### 4.4 V/Oct DAC Output

```cpp
// MIDI note to DAC count
// MIDI_ROOT = 48 (C3 = 0V output)
// VOLT_RANGE = calibrated full-scale range of op-amp output (e.g. 4.0)

dacCount = (midiNote - MIDI_ROOT) * (4096.0 / (12.0 * VOLT_RANGE));
```

12-bit DAC over 4V range = ~0.98 mV/step. One semitone = ~33.3 mV. Resolution ≈ 34 steps/semitone — more than sufficient for clean V/Oct tracking.

### 4.5 Clock & Timing

- **Internal clock:** software timer from RV3 reading, 20–300 BPM
- **External clock:** rising edge on J1 advances sequencer one step; overrides internal clock when patched
- **Gate length:** fixed at 50% duty cycle of step length (adjustable firmware constant)
- **Button short press:** reset sequence to step 1
- **Button long press (>500 ms):** cycle arp order (Up → Down → Random → Walk → Up...)

### 4.6 Arp Orders

| Mode | Behaviour |
|---|---|
| Up | Chord tones ascending, wraps at top |
| Down | Chord tones descending, wraps at bottom |
| Random | Random chord tone each step (chaos layer applied on top) |
| Walk | ±1 scale degree from current step, with wrap |

### 4.7 Firmware Module Structure

| File | Responsibility |
|---|---|
| `main.ino` | Setup, main loop, button handling, clock tick dispatch |
| `scales.h/.cpp` | Scale definitions, `quantise()`, MIDI-to-DAC conversion |
| `arp.h/.cpp` | Arp sequence state machine, order modes, step advance |
| `generative.h/.cpp` | Chaos engine: pitch walk, octave jump, gate density |
| `dac_out.h/.cpp` | DAC write wrapper, calibration offset, `noteToDAC()` |
| `gate_out.h/.cpp` | Gate output timing, configurable duty cycle |
| `rgb_led.h/.cpp` | NeoPixel wrapper, scale-to-colour mapping, brightness |
| `clock.h/.cpp` | Internal clock (timer), external clock (interrupt on J1), BPM calc |

---

## 5. Calibration Procedure

1. Power the module from the Eurorack bus. Connect a precision multimeter to J3 (V/Oct out).
2. Hold the button for 3 seconds at power-on to enter calibration mode (LED flashes white).
3. Module outputs voltage for MIDI note 48 (C3 = target 0.000V).
4. Adjust R3 trim pot until multimeter reads 0.000V (±2 mV acceptable).
5. Module then outputs MIDI note 84 (C7 = target 4.000V). Verify — no further adjustment needed (gain set by fixed R1/R2).
6. Short-press button to exit calibration and save offset to non-volatile memory (RA4M1 EEPROM emulation).

> **Note:** Re-calibrate if moved to significantly different temperature environment, or after a firmware update that resets EEPROM.

---

## 6. Bill of Materials

| Ref | Component | Value / Part | Qty | Source |
|---|---|---|---|---|
| U1 | LDO Regulator | AMS1117-5.0 SOT-223 | 1 | JLCPCB Basic |
| U2 | Op-Amp | MCP6002T-I/SN SOIC-8 | 1 | JLCPCB Extended |
| U3 | MCU Board | Seeed XIAO RA4M1 | 1 | Seeed Studio ~$5 |
| D1 | Protection Diode | 1N5819 SOD-123 | 1 | JLCPCB Basic |
| D2, D3 | Clamp Diodes | BAT54 SOD-323 (x2 per jack) | 4 | JLCPCB Basic |
| D9 | 3mm LED (alt) | 3mm flat-top red/green | 1 | Tayda (optional) |
| D13 | RGB LED | WS2812B PLCC4 5.0x5.0mm | 1 | JLCPCB Basic |
| Q1 | NPN Transistor | 2N3904 SOT-23 | 1 | JLCPCB Basic |
| J1–J4 | 3.5mm Jack | PJ301M-12 | 4 | AliExpress |
| J5 | Power Header | 2x5 2.54mm shrouded | 1 | AliExpress |
| RV1–RV3 | Potentiometer | B100K Alpha RD901F 9mm | 3 | Tayda Electronics |
| SW1 | Tact Switch | 6x6mm 13mm SPST-NO | 1 | Tayda Electronics |
| R1 | Resistor | 10 kΩ 0402 | 1 | JLCPCB Basic |
| R2 | Resistor | 12.1 kΩ 0402 1% | 1 | JLCPCB Basic |
| R3 | Trim Pot | 100 kΩ 3296W | 1 | AliExpress |
| R4 | Resistor | 1 kΩ 0402 (V/Oct output) | 1 | JLCPCB Basic |
| R5 | Resistor | 330 Ω 0402 (WS2812B data) | 1 | JLCPCB Basic |
| R6 | Resistor | 1 kΩ 0402 (transistor base) | 1 | JLCPCB Basic |
| R7 | Resistor | 10 kΩ 0402 (gate pull-up) | 1 | JLCPCB Basic |
| R8, R9 | Resistors | 3.3 kΩ 0402 (jack protection) | 2 | JLCPCB Basic |
| R10, R11 | Resistors | 100 kΩ 0402 (CV divider) | 2 | JLCPCB Basic |
| C1 | Capacitor | 100 nF 0402 (op-amp supply) | 1 | JLCPCB Basic |
| C2 | Capacitor | 10 nF 0402 (op-amp output) | 1 | JLCPCB Basic |
| C3 | Capacitor | 100 µF 6.3V electrolytic | 1 | JLCPCB Basic |
| C4 | Capacitor | 10 µF 6.3V electrolytic | 1 | JLCPCB Basic |
| C5, C6 | Capacitors | 100 nF 0402 (rail decoupling) | 2 | JLCPCB Basic |
| C7 | Capacitor | 100 µF 16V electrolytic | 1 | JLCPCB Basic |
| C8 | Capacitor | 100 nF 0402 (-12V decoupling) | 1 | JLCPCB Basic |
| — | XIAO Socket | 7-pin 2.54mm female header | 2 | Tayda Electronics |
| — | Panel | 4HP PCB panel (JLC fabricated) | 1 | JLCPCB |
| — | Knobs | Spline-shaft knob for 9mm pot | 3 | Tayda / AliExpress |
| — | Mounting screws | M3 x 6mm | 2 | Local |

---

## 7. Open Questions & Next Steps

### 7.1 Immediate (KiCad)

- [ ] Generate KiCad schematic (`.kicad_sch`) from this spec
- [ ] Place components and route PCB with symmetric layout constraint enforced
- [ ] Generate panel SVG / DXF for front panel fabrication
- [ ] Run DRC and ERC; fix violations
- [ ] Export Gerbers + BOM + CPL for JLCPCB quote

### 7.2 Firmware (Post-Prototype)

- [ ] Port scale quantiser and arp engine to Arduino C++
- [ ] Implement calibration mode and EEPROM storage
- [ ] Tune chaos probabilities for musical feel
- [ ] Test V/Oct accuracy against calibrated reference (target ±2 cents/octave)
- [ ] Investigate USB MIDI out (RA4M1 supports USB 2.0, zero hardware cost)

### 7.3 Design Decisions Pending

- [ ] Verify RA4M1 DAC0 is broken out to XIAO pin D0 — check Seeed schematic
- [ ] Bench-verify op-amp gain resistor values with actual DAC output
- [ ] Panel finish — matte black PCB panel vs anodised aluminium
- [ ] Confirm WS2812B vs 3mm LED choice for Rev 1 (recommend WS2812B)

---

## 8. Stretch Goals

### 🌟 Orientation-Sensing Reversible Panel

Wire GPIO D4 (currently reserved as I2C SDA) to a pad exposed on the front panel via a spring contact or pogo pin behind the panel.

- **Orientation A:** pad touches a grounded PCB trace → D4 reads LOW
- **Orientation B:** pad is open → D4 reads HIGH (internal pullup)

Firmware reads this at boot and automatically remaps jack functions and LED colours to match the installed orientation. The silkscreen can show different labels on each face, and the module self-configures to match.

**Implementation cost:** single GPIO, one PCB trace, one panel-side copper pad. Zero BOM addition.

A future panel revision could use this to offer two distinct firmware personalities — e.g. arpeggiator on face A, Euclidean rhythm generator on face B.

### 🌟 Bipolar V/Oct Output (±5V, ~8 Octaves)

Replace MCP6002 (single-supply 5V) with TL072 or OPA2134 powered from ±12V bus. Add inverting summing stage to shift 0–3.3V DAC output to -5V–+5V range. The -12V rail is already brought to the PCB in Rev 1.

### 🌟 USB MIDI Output

RA4M1 has native USB 2.0. The Arduino Renesas core supports USB-MIDI via the `MIDIUSB` library. The module could simultaneously output V/Oct CV and transmit MIDI note messages over USB — firmware change only, no hardware addition.

---

## 9. References

- [Hagiwo MOD2 documentation](https://note.com/solder_state/n/nce8f7defcf98)
- [WGD Modular Melon](https://wgdmodular.de/module/melon/)
- [Modulove MOD2 firmware repo](https://github.com/modulove/MOD2)
- [Seeed XIAO RA4M1 wiki](https://wiki.seeedstudio.com/XIAO_RA4M1/)
- Renesas RA4M1 datasheet — DAC chapter (section 37)
- MCP6002 datasheet — Microchip Technology
- [Daisy Seed specs](https://daisy.audio/hardware/Seed/) (considered, not used)
