# Story 006: Tempo Pot

**As a** patcher
**I want** to control the arpeggiator's tempo with a pot
**So that** I can play in time with other modules and vary the feel without reflashing firmware

## Acceptance Criteria
- [x] `firmware/arp/lib/tempo/` with pure-logic tempo module, no `<Arduino.h>` includes
  - `bpmFromPot(float pot)` — exponential mapping: pot=0 → 40 BPM, pot=1 → 300 BPM, doubles every 1/3 of rotation
  - `stepMsFromBpm(int bpm)` — inverse mapping, clamped to valid BPM range
- [x] Host unit tests under `firmware/arp/test/test_tempo/`: 15 GoogleTest cases covering endpoints, clamping, exponential curve, monotonicity, geometric mean midpoint, and stepMs inverse
- [x] Main loop reads pot on D5 and uses live BPM
- [x] BPM refers to *beats per minute*, not notes per minute — arp runs at 16th-note subdivision (4 steps per beat)
- [x] CI runs all host tests and stays green *(47 tests: 15 tempo + 9 arp + 23 scales)*

## Findings

### Spec pin-assignment errors
- Spec §2.3 assigns RV3 (tempo) to **D8/A8**, but **D8 is not ADC-capable** on the XIAO RA4M1. Only D0, D1, D2, D3, D5 support `analogRead`. Remapped tempo pot to D5 (was reserved as I2C SCL; I2C is unused in this design).
- The full §2.3 pin table needs revisiting: any pot or CV input on D4, D6, D7, D8, D9, or D10 will not work.

### BPM semantics
Initial implementation treated BPM as "steps per minute" which made the arp feel too slow at any musical tempo. Corrected to 16th-note subdivision: `stepMs = beatMs / 4`. At 120 BPM, each arp step is 125 ms.

## Depends on
- Story 005

## Status
done (bench verified 2026-04-18)

Pot on D5 sweeps tempo from 40 BPM (slow, ~375 ms/step) to 300 BPM (fast, ~50 ms/step) with exponential curve. Each third of the rotation roughly doubles the tempo.
