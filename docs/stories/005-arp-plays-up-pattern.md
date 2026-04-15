# Story 005: Arp Plays Up-Pattern (v0.1)

**As a** patcher
**I want** to patch the module's V/Oct and Gate outputs into a VCO + VCA + envelope and hear a looping 4-note up-arpeggio in the major scale
**So that** the module is demonstrably a working musical instrument, end to end

This story defines v0.1 — the minimal "it makes music" milestone.

## Acceptance Criteria — Firmware
- [ ] `firmware/arp/lib/arp/` with pure-logic arp state machine, no `<Arduino.h>` includes
  - Public API: `advance()` returns the next MIDI note in the sequence; `reset()` returns to step 0
  - Order: Up only. 4-note fixed chord (root, major 3rd, 5th, octave)
  - Root note is a `constexpr` (MIDI note 48 / C3 per spec §4.4)
- [ ] Host unit tests under `firmware/arp/test/native/test_arp/`:
  - Sequence is correct for 2 full cycles (stepping past the last note wraps to step 0)
  - `reset()` returns to step 0 regardless of current state
- [ ] Main loop integrates:
  - Internal clock at 120 BPM (fixed; from `constexpr`, not RV3)
  - On each clock tick: `arp.advance()` → `quantize()` → `noteToDAC()` → DAC write → gate high
  - Gate goes low at 50% of step length
- [ ] Gate output drives the NPN transistor stage (breadboarded or perfboard)
- [ ] CI runs all `test_scales` and `test_arp` host tests and stays green

## Acceptance Criteria — Bench
- [ ] Patching V/Oct → VCO V/Oct and Gate → envelope trigger, into a VCA audio output: an audible 4-note up-arpeggio loops continuously
- [ ] Pitch tracks within ±5¢ on every note (regression check on Story 004)
- [ ] Gate timing on DS1054Z shows 50% duty cycle, rise/fall <1 µs (spec §2.6)
- [ ] Short video or audio recording linked from `docs/bench-log.md`

## Notes
- Hardcoded root, tempo, and chord are deliberate. RV1/RV2/RV3 wiring exists but values are ignored. Chaos, other scales, arp orders, CV input, NeoPixel, button, EEPROM are all post-MVP stories.
- Gate length as 50% of step length is documented in spec §4.5. A longer/shorter constant can be tried if the envelope retrigger feels wrong — note the actual value used.
- On merge of this story, tag the firmware `arp/v0.1.0` (scoped SemVer per decisions.md §12) and rotate the `Unreleased` CHANGELOG entries under the new heading.

## Depends on
- Story 004

## Status
not started
