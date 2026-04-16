# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and firmware in this repo follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html). Hardware uses `Rev N.M` notation, silkscreened on the PCB. See `docs/decisions.md` §12 for the full scheme.

**Convention:** this file is updated on every commit. Changes in-flight go under `## [Unreleased]`; on a firmware tag (`arp/vX.Y.Z`), those entries move under a new `## [arp/vX.Y.Z] — YYYY-MM-DD` heading.

Section keys: `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`, `Docs`.

---

## [Unreleased]

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
