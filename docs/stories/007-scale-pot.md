# Story 007: Scale Pot

**As a** patcher
**I want** to change the arpeggiator's scale with a pot
**So that** I can shift the module's mood from bright major to moody minor to ambiguous pentatonic without touching firmware

## Acceptance Criteria
- [x] `scaleFromPot(pot, current)` added to `lib/scales/` — maps [0.0, 1.0] to one of 6 scales with boundary hysteresis
- [x] Pot range divided into 6 equal zones: Major, NaturalMinor, PentatonicMajor, PentatonicMinor, Dorian, Chromatic
- [x] ±2% hysteresis at each zone boundary prevents ADC jitter from flipping scales mid-note
- [x] Host unit tests under `test/test_scales/`: 10 new GoogleTest cases (zone centres, clamping, hysteresis both directions, sweep visits all 6 in order, boundary jitter doesn't flip)
- [x] Main loop reads pot on D2/A2 per spec §2.3, updates active scale each loop iteration
- [x] CI runs all host tests and stays green *(57 tests: 15 tempo + 9 arp + 33 scales)*

## Acceptance Criteria — Bench
- [x] Pot on D2/A2 sweeps audibly through 6 scales from CCW to CW *(bench verified 2026-04-18 — audible scale changes; most obvious where the E becomes E♭ in minor/Dorian/pentatonic-minor)*
- [x] No audible scale "wobble" at zone boundaries when pot held still *(hysteresis working as designed)*
- [x] Arp continues in time; scale change doesn't interrupt clocking

## Bench notes

The static C-E-G-C chord input makes some scale transitions subtler than others:
- Most audible: Major ↔ Natural Minor / Dorian / Pentatonic Minor (E → E♭)
- Subtle: Major ↔ Pentatonic Major (identical in this chord — both contain C, E, G)
- Inaudible: Major ↔ Chromatic (no quantisation change when input is already in major)

Scale changes will become much more expressive once chaos is introduced (Story 008) — the quantiser will have random-ish input notes to bend toward each scale's character.

## Notes
- Hysteresis is ±2% of pot rotation (0.02 normalised). 14-bit ADC noise is typically <0.2% with decent wiring, so 2% is a conservative margin.
- Scale is read every main-loop iteration, not just on change. Cheap — `scaleFromPot` is O(1) and `analogRead` is <10 µs.
- No power-on persistence: scale defaults to Major on boot regardless of pot position. First pot read in `loop()` corrects it within one step.

## Depends on
- Story 005 (arp integration)
- Story 006 (pattern for reading a pot and wiring through the scales library)

## Status
done (bench verified 2026-04-18)

Pot on D2/A2 cycles through 6 scales with audible character changes. Hysteresis stable at boundaries. Most obvious distinctions land on the chord's 3rd (E → E♭ in minor variants); expressiveness will broaden with Story 008 chaos.
