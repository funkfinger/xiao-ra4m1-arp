# Story 003: DAC Ramp and Scale Quantiser

**As a** bench engineer and developer
**I want** (a) the XIAO DAC to emit a visually clean linear voltage ramp, and (b) a pure-logic scale quantiser with host unit tests
**So that** the one novel hardware subsystem is validated in isolation and the first piece of musical logic exists with TDD scaffolding in place

Bundled in one PR because they are independent in code but share the same milestone: "novel subsystems proved, on their own terms."

## Acceptance Criteria — DAC Ramp (bench)
- [x] Firmware writes a 0→4095 ramp to DAC0 in a loop at ~10 Hz (so the scope can trigger on a rising edge) *(measured 8.56 Hz — overhead acceptable)*
- [x] Rigol DS1054Z shows 0 V → ~3.3 V linear sweep on DAC0 pin, no steps missing, no glitches beyond expected 12-bit resolution *(scope screenshot: `requirements/DS1Z_QuickPrint1.png`)*
- [x] Measured full-scale voltage and linearity recorded in `docs/bench-log.md` (create if missing) with date and scope screenshot reference
- [x] No op-amp yet — this measures the raw DAC pin only

## Acceptance Criteria — Scale Quantiser (host TDD)
- [x] `firmware/arp/lib/scales/` with `scales.h` / `scales.cpp`, no `<Arduino.h>` includes
- [x] Public API: `uint8_t quantize(uint8_t midiNote, Scale scale)` returning the nearest in-scale MIDI note. On equidistant ties (common in whole-tone gaps of major/minor), rounds *down* to the lower tone. Semantic is "nearest," not "floor" — pentatonic D in C-pentatonic-minor maps to E♭, not C.
- [x] All 6 scales from spec §4.3 defined: Major, Natural Minor, Pentatonic Major, Pentatonic Minor, Dorian, Chromatic
- [x] Tests under `firmware/arp/test/test_scales/` run via `pio test -e native`: *(23 tests, all pass)*
  - Each scale's in-scale notes pass through unchanged (property test or enumeration)
  - Out-of-scale notes map to the nearest in-scale tone (equidistant ties broken downward)
  - Chromatic is identity for all 128 MIDI notes
  - Octave invariance: `quantize(n, s) + 12 == quantize(n + 12, s)` for all n in range
- [x] Tests are written first; commits show red-green progression or at least a test-before-impl sequence
- [x] CI runs `pio test -e native` and stays green *(pending CI run on PR)*

## Notes
- The DAC ramp and quantiser share a PR because they're the two novel things PR 3 proves out. If it's easier to split, that's fine — update `decisions.md` bring-up table accordingly.
- `quantize()` signature may evolve (e.g., returning a struct with semitone offset). Start simple; refactor when the arp engine needs more.
- Tie-breaking rule (round down) is a convention — document it in the header so future chaos code can reason about it.
- Test file path changed from `test/native/test_scales/` to `test/test_scales/` per PlatformIO convention.
- GoogleTest required explicit `main()` in the test file — PlatformIO 6.1.18 doesn't auto-link `gtest_main`. Noted as a quirk.

## Depends on
- Story 002

## Status
done (PR #2, bench verified 2026-04-15)

DAC ramp: 0→3.3 V linear sawtooth at 8.56 Hz, no glitches, scope-confirmed.
Scale quantiser: 23 GoogleTest cases (5 passthrough × scales, chromatic identity, 5 octave invariance, tie-break, nearest-not-floor, boundaries, 5 output-always-in-scale). All pass in 1.45s on host.
