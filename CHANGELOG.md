# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and firmware in this repo follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html). Hardware uses `Rev N.M` notation, silkscreened on the PCB. See `docs/decisions.md` §12 for the full scheme.

**Convention:** this file is updated on every commit. Changes in-flight go under `## [Unreleased]`; on a firmware tag (`arp/vX.Y.Z`), those entries move under a new `## [arp/vX.Y.Z] — YYYY-MM-DD` heading.

Section keys: `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`, `Docs`.

---

## [Unreleased]

### Added (Story 008 — Arp orders)
- `Order` enum added to `lib/arp/` with 6 values: Up, Down, UpDown, DownUp, Skip, Random
- Pattern table driven — any pattern length supported (4 and 6 used today). Rate-independent timing (each step = 16th note, odd-length patterns produce polyrhythms against 4/4)
- Random order uses an internal LCG seeded at boot from `millis()`; deterministic for host tests
- `setOrder()` restarts pattern from step 0; `setSeed()` reseeds Random
- `orderFromPot(float, Order)` — pot-to-order mapping with ±2% boundary hysteresis, mirrors `scaleFromPot`
- 14 new GoogleTest cases covering all orders, deterministic Random, setOrder restart behaviour, in-chord invariants across all orders, pot zone mapping, hysteresis
- `src/main.cpp` reads pot on D3/A3 each loop iteration, calls `setOrder()` when value changes
- Total host tests: 71 (15 tempo + 23 arp + 33 scales)

### Changed
- Spec §2.3 assigned D3/A3 to RV2 (chaos); repurposed temporarily for arp-order selection while chaos design is still open. Documented for revisit when chaos lands or encoder UI is adopted.

### Added (Story 007 — Scale pot)
- `scaleFromPot(float pot, Scale current)` added to `lib/scales/` — pure-logic pot-to-scale mapping with 6 equal zones and ±2% boundary hysteresis
- `kScaleHysteresis = 0.02f` — hysteresis constant exposed in header
- 10 new GoogleTest cases: zone centres, clamping, hysteresis both directions, sweep visits all 6 in order, boundary jitter doesn't flip scales
- `src/main.cpp` — reads scale pot on D2/A2 per spec §2.3, updates active scale each loop iteration
- Total host tests: 57 (15 tempo + 9 arp + 33 scales)

### Docs
- Spec revised to Rev 0.1a with bench-verified corrections from Stories 003–006. No PCB produced yet so no physical Rev change; Rev 0.1a is a paper revision. Changes marked inline and summarised in new "Spec Revision History" section at the top of `docs/generative-arp-module.md`:
  - §2.3 pin table — RV3 (tempo) moved from D8/A8 to D5 (D8 is not ADC-capable); D4 and D8 become reserved future-expansion pins
  - §2.4.2 CV input — own ADC channel (D1/A1); divider ratio corrected 100 k / 220 k
  - §2.5.1 output range — C3–C7 (0–4 V), was C2–C6 which requires -1 V
  - §2.5.2 op-amp — R2 = 2.7 kΩ (gain 1.27×), was 12.1 kΩ (gain 2.21×, maxes at 7.3 V); R3 removed; R4 = 100 Ω (was 1 kΩ, caused V/Oct loading error)
  - §4.4 semitone voltage — 83.3 mV, was 33.3 mV
  - §4.5 tempo — 40–300 BPM exponential, 16th-note subdivision
  - §6 BOM — R2, R3, R4 updated

### Added (Story 006 — Tempo pot)
- `firmware/arp/lib/tempo/tempo.h` / `tempo.cpp` — pure-logic tempo mapping: `bpmFromPot(float)` with exponential curve (40–300 BPM, doubles every 1/3 of rotation), `stepMsFromBpm(int)` inverse with clamping
- `firmware/arp/test/test_tempo/test_tempo.cpp` — 15 GoogleTest cases covering endpoints, clamping, exponential curve, monotonicity, geometric mean midpoint, stepMs
- `firmware/arp/src/main.cpp` — reads tempo pot on D5, uses live BPM for step timing, 16th-note subdivision (4 steps per beat)

### Fixed
- Tempo pot remapped from D8 (spec §2.3) to D5 — D8 is not ADC-capable on XIAO RA4M1. Only D0, D1, D2, D3, D5 support `analogRead`. Spec §2.3 pin table needs full revision.
- BPM semantics corrected: each arp step is now a 16th note (beatMs / 4), not a full beat. Arp feels musically correct at any BPM.

---

## [arp/v0.1.0] — 2026-04-16

First playable firmware. 4-note up-arpeggio (C3-E3-G3-C4) at 120 BPM with calibrated V/Oct and gated output. Breadboard-verified through Mutable Instruments Plaits.

### Added
- Repo scaffold: `firmware/arp/`, `hardware/`, `docs/` directory tree with placeholder READMEs in empty dirs
- Three license files at repo root: `LICENSE-firmware` (MIT), `LICENSE-hardware` (CERN-OHL-S v2), `LICENSE-docs` (CC-BY-SA 4.0)
- Root `README.md` — project summary, status, toolchain, versioning, license map
- `.gitignore` covering PlatformIO, KiCad, editor, build artefact, and macOS metadata
- `.github/workflows/ci.yml` — GitHub Actions job installing PlatformIO, running `pio test -e native` when a project exists, passing vacuously otherwise

### Docs
- Initial spec dropped in: `docs/generative-arp-module.md` (Rev 0.1, April 2026)
- Design & process decisions captured in `docs/decisions.md` from 2026-04-14 grill-me session — monorepo layout, PlatformIO toolchain, minimal v0.1 scope, deferred KiCad, host TDD + bench verification, licensing, CI, git workflow
- User-story methodology adopted; `docs/stories/` seeded with stories 001–005 covering the MVP bring-up sequence
- `CHANGELOG.md` started with Keep a Changelog format; convention to update on every commit
- Versioning scheme adopted: SemVer for firmware with scoped tags (`arp/vX.Y.Z`), `Rev N.M` for hardware (see decisions.md §12)
- Story 001 completed — repo scaffolded, initial commit pushed to GitHub

### Changed
- Hardware license swapped from CERN-OHL-S v2 (strongly reciprocal copyleft) to Apache 2.0 (permissive)
- Docs license swapped from CC-BY-SA 4.0 (share-alike) to CC-BY 4.0 (attribution only)
- All three licenses (firmware MIT, hardware Apache 2.0, docs CC-BY 4.0) are now permissive — no copyleft or share-alike obligations on derivatives
- Corrected board identifier from `seeed_xiao_ra4m1` (underscores) to `seeed-xiao-ra4m1` (hyphens) — matches the platform manifest in `Seeed-Studio/platform-seeedboards`

### Added (Story 002 — XIAO blinks)
- `firmware/arp/platformio.ini` — Seeed platform fork, `seeed-xiao-ra4m1` board, Arduino framework
- `firmware/arp/src/main.cpp` — minimal 1 Hz blinky toggling `LED_BUILTIN` (active-LOW)
- `firmware/arp/README.md` expanded with build, upload, and DFU recovery instructions
- `.github/workflows/ci.yml` updated: compile firmware via `pio run -d firmware/arp`, run host tests conditionally when a `test/` dir and `[env:native]` exist
- Story 002 bench-verified 2026-04-15 — onboard LED blinks at 1 Hz on physical XIAO RA4M1; DFU upload 15 s, 33 464 bytes

### Added (Story 003 — DAC ramp + scale quantiser)
- `firmware/arp/lib/scales/scales.h` / `scales.cpp` — pure-logic scale quantiser with 6 scales (Major, Natural Minor, Pentatonic Major/Minor, Dorian, Chromatic). Nearest-note semantics with equidistant tie-break downward. No Arduino dependencies.
- `firmware/arp/test/test_scales/test_scales.cpp` — 23 GoogleTest cases covering in-scale passthrough, chromatic identity, octave invariance, tie-break direction, nearest-not-floor pentatonic behaviour, boundary values, output-always-in-scale invariants
- `firmware/arp/platformio.ini` — `[env:native]` added for host-side GoogleTest; `test_ignore = *` on embedded env
- `firmware/arp/src/main.cpp` — DAC ramp: 0→4095 linear sawtooth at ~10 Hz on DAC0 (D0/A0)
- `docs/bench-log.md` — DAC ramp measurements: 0→3.3 V, 8.56 Hz, linear, no glitches (scope screenshot: `requirements/DS1Z_QuickPrint1.png`)
- `requirements/DS1Z_QuickPrint1.png` — Rigol DS1054Z scope capture of DAC ramp

### Added (Story 004 — V/Oct tracks across octaves, in progress)
- `docs/calibration.md` — op-amp circuit spec (MCP6002, gain 1.27× with R2=2.7k/R1=10k), wiring table, target voltage table, and bench measurements
- `firmware/arp/src/main.cpp` — V/Oct calibration firmware stepping C3–C7 (0–4V) with serial output; gain corrected to 1.261 from bench measurement
- `requirements/DS1Z_QuickPrint3.png` — scope capture of V/Oct staircase (5 levels, 0–4V, 2s/step)
- Spec §2.5.2 gain error identified: stated R2=12.1k (gain 2.21×) but correct gain for 0–3.3V → 0–4V is 1.27× (R2=2.7k); documented in calibration.md
- Bench-verified voltages within ±2 mV after gain correction
- Pitch tracking verified via Plaits + tuner: ±0.5¢ upper 3 octaves, -5.4¢ lowest octave (tuner precision limit)
- R4 (1kΩ) loading issue identified and documented — bypassed on breadboard; PCB note: reduce to 100Ω or remove
- Spec §2.5.2 gain error documented: 2.21× incorrect, correct gain is ~1.27× for 0–4V range
- Story 004 complete

### Added (Story 005 — Arp plays up-pattern, v0.1)
- `firmware/arp/lib/arp/arp.h` / `arp.cpp` — pure-logic arp state machine: up-order, 4-note chord (root, M3, 5th, octave), wrapping
- `firmware/arp/test/test_arp/test_arp.cpp` — 9 GoogleTest cases (sequence, wrap, reset, different roots, step tracking)
- `firmware/arp/src/main.cpp` — integrated arp: 120 BPM internal clock, quantised V/Oct output, 50% gate duty via BC548C NPN
- `requirements/DS1Z_QuickPrint4.png` — scope capture of V/Oct staircase (4 levels, 120 BPM)
- `requirements/DS1Z_QuickPrint5.png` — scope capture of V/Oct + gate dual-channel
- `requirements/arp.m4a` — audio recording of working arpeggiator via Plaits

### Fixed
- Gate polarity: NPN common-emitter inverts signal; swapped firmware HIGH/LOW so gate output is HIGH (5V) during note-on
