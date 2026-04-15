# Story 004: V/Oct Tracks Across Octaves

**As a** bench engineer
**I want** the DAC → op-amp signal chain, breadboarded, to produce a V/Oct output that tracks within ±5¢ across C2–C6
**So that** the module can drive a VCO musically, proving the spec §2.5 scaling circuit works with real components before committing to a PCB

## Acceptance Criteria
- [ ] MCP6002 (DIP-8) breadboarded per spec §2.5.2 with R1=10k, R2=12.1k (or nearest E96 stock values — record what was actually used)
- [ ] Firmware outputs a loop of MIDI notes C2, C3, C4, C5, C6 on DAC0, one per second, repeating indefinitely
- [ ] Signal chain: XIAO DAC0 → MCP6002 in → MCP6002 out → jack → Plaits (or Castor & Pollux) V/Oct in
- [ ] Target-voltage table committed to `docs/calibration.md` listing expected op-amp output voltage for each of C2–C6 (based on final measured gain)
- [ ] Multimeter verifies each voltage is within ±2 mV of the table entry
- [ ] Pitch at the VCO audio out tracks within ±5¢ at each of C2, C3, C4, C5, C6 (measured via tuner app or pedal tuner)
- [ ] Gain and any trim values recorded in `docs/calibration.md` with date, Plaits/C&P serial, and ambient temperature
- [ ] Calibration constants live in a clearly named `#define` or `constexpr` in firmware (post-v0.1 moves these to EEPROM — not this story)

## Notes
- If R1/R2 values from spec §2.5.2 produce gain that misses the target octave range, adjust R2 to whatever stock value is closest and record it. The *measured* gain goes in `calibration.md`, not the computed value.
- Single-supply MCP6002 cannot output true 0 V — expect ~10–50 mV offset at C2. Either trim it out (spec §2.5.2 R3 trim pot) or accept and document.
- ±5¢ is the MVP target. ±2¢ (spec §7.2) is post-MVP.

## Depends on
- Story 003

## Status
not started
