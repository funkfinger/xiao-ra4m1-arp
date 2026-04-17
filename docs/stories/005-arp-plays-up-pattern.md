# Story 005: Arp Plays Up-Pattern (v0.1)

**As a** patcher
**I want** to patch the module's V/Oct and Gate outputs into a VCO + VCA + envelope and hear a looping 4-note up-arpeggio in the major scale
**So that** the module is demonstrably a working musical instrument, end to end

This story defines v0.1 — the minimal "it makes music" milestone.

## Acceptance Criteria — Firmware
- [x] `firmware/arp/lib/arp/` with pure-logic arp state machine, no `<Arduino.h>` includes
  - Public API: `advance()` returns the next MIDI note in the sequence; `reset()` returns to step 0
  - Order: Up only. 4-note fixed chord (root, major 3rd, 5th, octave)
  - Root note is a `constexpr` (MIDI note 48 / C3 per spec §4.4)
- [x] Host unit tests under `firmware/arp/test/test_arp/`: *(9 tests, all pass)*
  - Sequence is correct for 2 full cycles (stepping past the last note wraps to step 0)
  - `reset()` returns to step 0 regardless of current state
- [x] Main loop integrates:
  - Internal clock at 120 BPM (fixed; from `constexpr`, not RV3)
  - On each clock tick: `arp.advance()` → `quantize()` → `noteToDAC()` → DAC write → gate high
  - Gate goes low at 50% of step length
- [x] Gate output drives BC548C NPN transistor stage (breadboarded) *(substituted for 2N3904; same pinout/function)*
- [x] CI runs all `test_scales` and `test_arp` host tests and stays green *(32 tests total)*

## Acceptance Criteria — Bench
- [x] Patching V/Oct → Plaits V/Oct and Gate → envelope trigger: an audible 4-note up-arpeggio loops continuously
- [x] Pitch tracks within ±5¢ on every note (regression from Story 004 — same calibrated gain)
- [x] Gate timing on DS1054Z shows 50% duty cycle *(scope screenshots: `requirements/DS1Z_QuickPrint4.png`, `DS1Z_QuickPrint5.png`)*
- [x] Audio recording: `requirements/arp.m4a` *(phone mic via Plaits)*

## Findings

### Gate inversion
NPN common-emitter circuit inverts the gate signal. Initial firmware had gate polarity backward (HIGH on D6 → collector LOW → 0V at gate output). Fixed by swapping: D6 LOW = gate on (collector HIGH = 5V), D6 HIGH = gate off (collector LOW = 0V).

### Component substitutions
- MCP6002-I/P used (spec part) — from Cabinet 3, Bin 32, Section C
- BC548C NPN used instead of 2N3904 — same pinout (E-B-C), higher hFE, from Cabinet 2, Bin 26
- R2=2.7kΩ (gain 1.27× nominal, 1.261 measured) instead of spec's 12.1kΩ
- R4 bypassed (loading issue from Story 004)

## Notes
- Hardcoded root, tempo, and chord are deliberate. RV1/RV2/RV3 wiring exists but values are ignored. Chaos, other scales, arp orders, CV input, NeoPixel, button, EEPROM are all post-MVP stories.
- Gate length as 50% of step length is documented in spec §4.5.
- On merge of this story, tag the firmware `arp/v0.1.0` (scoped SemVer per decisions.md §12) and rotate the `Unreleased` CHANGELOG entries under the new heading.

## Depends on
- Story 004

## Status
done (bench verified 2026-04-16)

32 host tests pass (9 arp + 23 scales). Audible 4-note C-E-G-C up-arpeggio at 120 BPM through Plaits. Gate and V/Oct confirmed on scope. v0.1 achieved.
