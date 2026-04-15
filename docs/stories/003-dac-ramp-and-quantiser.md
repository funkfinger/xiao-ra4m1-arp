# Story 003: DAC Ramp and Scale Quantiser

**As a** bench engineer and developer
**I want** (a) the XIAO DAC to emit a visually clean linear voltage ramp, and (b) a pure-logic scale quantiser with host unit tests
**So that** the one novel hardware subsystem is validated in isolation and the first piece of musical logic exists with TDD scaffolding in place

Bundled in one PR because they are independent in code but share the same milestone: "novel subsystems proved, on their own terms."

## Acceptance Criteria — DAC Ramp (bench)
- [ ] Firmware writes a 0→4095 ramp to DAC0 in a loop at ~10 Hz (so the scope can trigger on a rising edge)
- [ ] Rigol DS1054Z shows 0 V → ~3.3 V linear sweep on DAC0 pin, no steps missing, no glitches beyond expected 12-bit resolution
- [ ] Measured full-scale voltage and linearity recorded in `docs/bench-log.md` (create if missing) with date and scope screenshot reference
- [ ] No op-amp yet — this measures the raw DAC pin only

## Acceptance Criteria — Scale Quantiser (host TDD)
- [ ] `firmware/arp/lib/scales/` with `scales.h` / `scales.cpp`, no `<Arduino.h>` includes
- [ ] Public API: `uint8_t quantize(uint8_t midiNote, Scale scale)` returning the nearest in-scale MIDI note (rounds down on tie, documented)
- [ ] All 6 scales from spec §4.3 defined: Major, Natural Minor, Pentatonic Major, Pentatonic Minor, Dorian, Chromatic
- [ ] Tests under `firmware/arp/test/native/test_scales/` run via `pio test -e native`:
  - Each scale's in-scale notes pass through unchanged (property test or enumeration)
  - Out-of-scale notes map to the nearest lower scale tone
  - Chromatic is identity for all 128 MIDI notes
  - Octave invariance: `quantize(n, s) + 12 == quantize(n + 12, s)` for all n in range
- [ ] Tests are written first; commits show red-green progression or at least a test-before-impl sequence
- [ ] CI runs `pio test -e native` and stays green

## Notes
- The DAC ramp and quantiser share a PR because they're the two novel things PR 3 proves out. If it's easier to split, that's fine — update `decisions.md` bring-up table accordingly.
- `quantize()` signature may evolve (e.g., returning a struct with semitone offset). Start simple; refactor when the arp engine needs more.
- Tie-breaking rule (round down) is a convention — document it in the header so future chaos code can reason about it.

## Depends on
- Story 002

## Status
not started
