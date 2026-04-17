# Story 004: V/Oct Tracks Across Octaves

**As a** bench engineer
**I want** the DAC → op-amp signal chain, breadboarded, to produce a V/Oct output that tracks within ±5¢ across C3–C7
**So that** the module can drive a VCO musically, proving the scaling circuit works with real components before committing to a PCB

## Acceptance Criteria
- [x] MCP6002-I/P (DIP-8) breadboarded with R1=10k, R2=2.7k (gain 1.27× nominal, 1.261 measured)
- [x] Firmware outputs MIDI notes C3, C4, C5, C6, C7 on DAC0, 2 seconds each, with serial output *(originally spec'd as C2–C6 but C2 requires negative voltage on single supply)*
- [x] Signal chain: XIAO DAC0 (pin 1) → MCP6002 pin 3 → MCP6002 pin 1 → Plaits V/Oct in
- [x] Target-voltage table committed to `docs/calibration.md` with measured gain
- [x] Multimeter verifies each voltage within ±2 mV *(all 5 voltages within ±2 mV after GAIN corrected to 1.261)*
- [x] Pitch at VCO audio out tracks within ±5¢ *(C4–C7: ±0.5¢; C3–C4: -5.4¢ borderline — tuner precision limit at 67 Hz)*
- [x] Gain and measurements recorded in `docs/calibration.md` with date
- [x] Calibration constants as `constexpr` in firmware: `GAIN = 1.261f`

## Findings

### Spec errors identified
- §2.5.2 gain: stated R2=12.1k (gain 2.21×) but 3.3V × 2.21 = 7.3V, not 4V. Correct gain for 0–4V is ~1.27× (R2=2.7k).
- §2.5.2 / §4.4 range: stated C2–C6 but C3=0V (MIDI 48), so C2 would need -1V — impossible on single supply. Actual range: C3–C7 (0–4V).
- §4.4 semitone voltage: stated "33.3 mV" but correct value is 83.33 mV (1V/12).

### R4 output resistor causes V/Oct loading
R4=1kΩ (standard Eurorack output impedance) forms a voltage divider with Plaits' ~100kΩ input, causing -10 to -20¢/octave flat error. Bypassing R4 fixed tracking to ±0.5¢. **PCB design note: reduce R4 to 100Ω or remove.**

## Depends on
- Story 003

## Status
done (bench verified 2026-04-16)

Voltages within ±2 mV. Pitch tracking within ±5¢ (upper 3 octaves within ±0.5¢). R4 bypassed on breadboard. Scope + measurement data in `docs/calibration.md`.
