# Design & Process Decisions

Captured from the 2026-04-14 grill-me session. These settle scope, tooling, and workflow for the first revision of the module. Source of truth for decisions that aren't obvious from the code.

---

## Context

This repo hosts a 4 HP Eurorack generative arpeggiator module based on the XIAO RA4M1, as specified in [`generative-arp-module.md`](./generative-arp-module.md). It will likely grow to host additional firmwares for the same hardware platform (see stretch goal §8 in the spec). The repo may be renamed from `xiao-ra4m1-arp` to `xiao-ra4m1-module` once that future is committed to.

---

## Decisions

### 1. Monorepo with three top-level areas

```
xiao-ra4m1-{arp,module}/
├── firmware/arp/     PlatformIO project for the arpeggiator firmware
├── hardware/         KiCad project (schematic, PCB, panel) — added post-v0.1
└── docs/             Spec, decisions, stories
```

**Why:** hardware and firmware revs are tightly coupled on a single-PCB module. Pin assignments are the contract between them and should version together. A firmware tag should correspond to a hardware rev. Future firmwares (`firmware/euclidean/`, etc.) live alongside `arp/`.

### 2. PlatformIO as the build system

Using Seeed's platform fork: `platform = https://github.com/Seeed-Studio/platform-seeedboards.git`, `board = seeed_xiao_ra4m1`. The XIAO RA4M1 is not in PlatformIO's official registry, but Seeed's fork provides a working board definition bundled with `ArduinoCore-renesas`.

**Fallback:** Arduino IDE or `arduino-cli` if PlatformIO hits a wall.

**Why PlatformIO over Arduino IDE:** scriptable builds, `pio test -e native` for host-side unit tests, library management via `platformio.ini`, CI-friendly.

### 3. MVP (v0.1) is deliberately minimal

First working firmware:
- Internal clock only, fixed BPM (RV3 present but ignored)
- Major scale only (RV1 present but ignored)
- Up-arp, 4 notes (root, 3rd, 5th, octave)
- DAC → op-amp → V/Oct out, calibrated against Plaits or Castor & Pollux
- Gate out via NPN transistor stage
- No chaos, no CV in, no NeoPixel, no button, no EEPROM

**Why:** the one genuinely novel subsystem is RA4M1 DAC → MCP6002 → Eurorack V/Oct. Prove that before stacking features that all depend on it.

### 4. KiCad deferred until breadboard signal chain validates

Don't open KiCad until:
1. DAC → MCP6002 → multimeter matches the calibration voltage table within ±2 mV on breadboard
2. Gate transistor stage drives a real Eurorack input
3. All pin assignments from spec §2.3 actually work

**Why:** spec §2.5.2 computed gain (R1=10k, R2=12.1k → 2.21×) is calculated, not measured. Finding the right gain on breadboard is 30 seconds; finding it after PCB fab is a Rev 0.2 respin.

### 5. Test strategy: host TDD for pure logic, bench for HAL

Pure-logic modules (`scales`, `arp`, `generative`) are unit tested on the host via `pio test -e native`. These modules must not `#include <Arduino.h>` or call `analogRead()` — they take inputs and return outputs.

HAL modules (`dac_out`, `gate_out`, `rgb_led`, `clock`) are bench-verified — no host tests.

**Why:** the generative engine has invariants ("never emits out-of-scale note", "octave jumps within ±2") that are tedious to verify by ear but trivial on host with a seeded RNG. Test-first where feasible; don't spin wheels chasing 100% coverage on embedded code.

### 6. V/Oct verification: DAC → VCO → tuner

Bench workflow: DAC output → Plaits or Castor & Pollux V/Oct in → audio into a tuner app (phone or pedal). Target ±5¢ cents-per-octave tracking for MVP (spec aims for ±2¢ eventually).

Rigol DS1054Z covers gate edges, DAC settling time, any signal glitches.

### 7. Licensing

| Area | License |
|---|---|
| Firmware | MIT |
| Hardware (KiCad, panel artwork) | Apache 2.0 |
| Docs | CC-BY 4.0 |

**Why:** all three are permissive — derivative works, commercial use, and closed-source forks are allowed with attribution. No share-alike obligations. Originally considered CERN-OHL-S v2 + CC-BY-SA 4.0 (the Hagiwo/Modulove/WGD ecosystem default) but swapped to permissive across the board so downstream use faces no copyleft constraint.

### 8. CI: GitHub Actions running host tests

Minimal `.github/workflows/ci.yml` runs `pio test -e native` on push and PR. No firmware build matrix yet (not needed until multiple firmwares exist). Add firmware compile-check when it saves time.

### 9. Git workflow: feature branches + self-reviewed PRs

Even solo. CI gates PRs, self-review catches real bugs, history stays readable. Cost ≈ 30 seconds per change.

### 10. User stories as the unit of work

See [`stories/README.md`](./stories/README.md) for format. Each story defines a user-visible or developer-visible outcome with acceptance criteria. Stories are satisfied by code and, where feasible, by tests.

### 11. CHANGELOG.md updated on every commit

[`CHANGELOG.md`](../CHANGELOG.md) at the repo root follows the [Keep a Changelog](https://keepachangelog.com/) format. In-flight work goes under `## [Unreleased]`; on a tag, those entries move under a new versioned heading. Every commit — not just releases — appends to Unreleased.

**Why:** the spec and the stories describe intent; the changelog describes what actually landed, in chronological order, without having to read `git log`. Keeps the reader-facing history honest and pairs cleanly with feature-branch PRs (each PR's description becomes the changelog entry).

### 12. Versioning — SemVer for firmware (scoped), Rev N.M for hardware

**Firmware: [Semantic Versioning 2.0.0](https://semver.org/).** Tags are scoped by firmware name so future firmwares in the same repo don't collide: `arp/v0.1.0`, `arp/v0.2.0`, `euclidean/v0.1.0`, etc.

- **MAJOR** — breaking change to EEPROM layout, calibration data format, or established user-facing behaviour
- **MINOR** — additive feature (a new scale, new arp order, CV input support) with no breaking change
- **PATCH** — bug fix, timing tweak, internal refactor with no user-visible change
- **Pre-1.0** (`0.x.y`) — SemVer explicitly allows anything-goes here; we use this phase through the first few playable revisions. First tag is `arp/v0.1.0` landing with Story 005.
- **1.0.0** — first release we consider complete, documented, reproducible by another hobbyist.

**Hardware: Rev N.M.** Matches spec §2.1 notation and PCB industry convention. `Rev 0.1` is the first fabricated board after breadboard signal-chain validation; `Rev 1.0` is the first publicly-documented release. Rev is silkscreened on the PCB.

**Firmware ↔ hardware compatibility** is recorded in the firmware's `CHANGELOG.md` entry and in the README (e.g., "arp/v0.3.0 supports Rev 0.1 and Rev 0.2"). Firmware MAJOR bumps when a hardware rev requires code changes that break compatibility with older boards.

**Docs: unversioned.** The `Rev 0.1` in the spec header refers to the *hardware* rev the doc describes, not the doc itself. Docs evolve in-place; history lives in `git log`.

---

## Bring-up sequence (first five PRs)

Each PR is landable independently. Each corresponds to a story in [`stories/`](./stories/).

| PR | Story | Outcome |
|---|---|---|
| 1 | [001](./stories/001-repo-scaffolded.md) | Repo skeleton, licenses, CI, `.gitignore`, empty `firmware/arp/` and `hardware/` |
| 2 | [002](./stories/002-xiao-blinks.md) | `firmware/arp/` builds and flashes a blinky to the XIAO |
| 3 | [003](./stories/003-dac-ramp-and-quantiser.md) | DAC emits linear ramp (scope-verified); scale quantiser TDD'd on host |
| 4 | [004](./stories/004-voct-tracks-octaves.md) | Op-amp breadboard + calibration; V/Oct tracks ±5¢ across C2–C6 |
| 5 | [005](./stories/005-arp-plays-up-pattern.md) | Minimal arpeggiator — **v0.1 makes music** |

After PR 5, everything else (chaos, six scales, CV in, NeoPixel, button, EEPROM, KiCad) is additive.

---

## Deferred decisions

These were intentionally left unresolved in the grill-me. Noted so they aren't forgotten.

- **RNG choice.** Arduino `random()` fine for MVP. Post-MVP may want a seedable PRNG (`std::minstd_rand` or XORShift) for reproducible chaos and for host-side statistical tests of the generative engine.
- **Calibration storage.** Hardcoded `#define` in v0.1. Move to RA4M1 EEPROM emulation when button + calibration mode lands (post-MVP).
- **USB MIDI.** Stretch goal (spec §8). No pin reservation needed — USB is hardwired on the XIAO. Decide when we want it.
- **Panel tooling.** Deferred with KiCad. Likely KiCad-native panel fab via JLCPCB.
- **Parts-on-hand audit.** Before PR 4, confirm MCP6002 (DIP-8 for breadboard), 2N3904, resistor values, and breadboarding hardware are physically present. Order from Digi-Key / Mouser if not (~2 day lead time vs 2 weeks AliExpress).
