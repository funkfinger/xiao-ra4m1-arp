# Story 008: Arp Orders

**As a** patcher
**I want** to change the arpeggiator's stepping pattern with a pot
**So that** the rhythmic character of the arp can vary without stopping playback

## Acceptance Criteria
- [x] `Order` enum added to `lib/arp/` with 6 values: Up, Down, UpDown, DownUp, Skip, Random
- [x] Existing `Up` behaviour preserved (default; all v0.1 tests still pass)
- [x] Patterns stored as index arrays; any pattern length supported (4 for Up/Down/Skip, 6 for UpDown/DownUp)
- [x] Rate-independent timing: each pattern step is one 16th note regardless of pattern length — odd-length patterns create polyrhythms against 4/4
- [x] Random uses internal LCG, deterministic given seed; main loop seeds at boot from `millis()`
- [x] `orderFromPot(pot, current)` — pot-to-order mapping with ±2% boundary hysteresis, mirrors `scaleFromPot` pattern
- [x] Main loop reads pot on D3/A3, calls `arpeggiator.setOrder()` when value changes
- [x] 22 GoogleTest cases cover all orders, pattern determinism, setOrder restart, Random in-chord invariant, pot mapping, hysteresis
- [x] Total host tests: 71 (15 tempo + 23 arp + 33 scales)
- [x] CI green

## Acceptance Criteria — Bench
- [x] Pot on D3/A3 cycles audibly through 6 orders CCW → CW (bench verified 2026-04-18)
- [x] UpDown/DownUp 6-step patterns create audible polyrhythm against 4/4 timing
- [x] Random changes every note, stays within the chord

## Design Decisions

### Rate-independent pattern timing
Each arp step is a 16th note regardless of how many steps the pattern has. A 6-step pattern at 120 BPM cycles every 750 ms (1.5 beats), producing a generative polyrhythm against 4/4. This matches how Arturia Keystep, Korg SQ-1, and most modern arps behave. An encoder-driven "pattern-length override" (to force short patterns into longer cycles, or fit odd patterns into musical bars) is a future option once the encoder UI lands.

### Pot repurposed from chaos
Spec §2.3 assigned D3/A3 to RV2 (chaos). This story repurposes it temporarily to arp-order selection while chaos design remains open-ended. When chaos is implemented, the order control likely moves to the button or the planned rotary encoder; D3 can then return to chaos duty. Documented in memory: `rotary_encoder_reconsideration.md`.

### Random uses LCG seeded at boot
Deterministic given seed (essential for host tests) and cheap on hardware. Main loop seeds with `millis() | 1` to avoid zero-state at boot. This is an MVP choice; a better-quality PRNG (XOR-shift, Mersenne) is a post-v0.2 concern.

### setOrder restarts pattern from step 0
Prevents confusing mid-pattern jumps when the pot crosses a boundary. Step counter resets; Random keeps its RNG state.

## Depends on
- Story 005 (arp base)
- Story 007 (established pot-reading + hysteresis pattern)

## Status
done (bench verified 2026-04-18)

6 arp orders live on D3/A3 pot with boundary hysteresis. Polyrhythmic 6-step patterns (UpDown/DownUp) work as expected. Random stays in the chord. Order change doesn't interrupt clocking.
