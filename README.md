# xiao-ra4m1-arp

A 4 HP Eurorack generative arpeggiator module built around the Seeed XIAO RA4M1. Produces scale-quantised V/Oct pitch CV and gate outputs driven by a probability-layered generative engine. Open-source hardware and firmware.

Inspired by [Hagiwo's MOD2](https://note.com/solder_state/n/nce8f7defcf98) and [WGD Modular's Melon](https://wgdmodular.de/module/melon/). Uses the RA4M1's onboard 12-bit DAC (via MCP6002 op-amp scaling) for real V/Oct pitch CV — no external DAC chip required.

## Status

**Planning / pre-prototype.** No hardware fabricated yet; no firmware released. See [`docs/decisions.md`](docs/decisions.md) for the bring-up plan and [`docs/stories/`](docs/stories/) for the story-driven development backlog.

- Current hardware rev: **none** (KiCad deferred until breadboard signal chain validates — see [decisions §4](docs/decisions.md))
- First firmware tag target: **`arp/v0.1.0`** on completion of [Story 005](docs/stories/005-arp-plays-up-pattern.md)

## Repo Layout

```
xiao-ra4m1-arp/
├── firmware/arp/     PlatformIO project for the arpeggiator firmware
├── hardware/         KiCad project — populated post-breadboard validation
├── docs/             Specification, decisions, user stories
└── CHANGELOG.md      Keep a Changelog format; updated on every commit
```

The repo may later host additional firmwares for the same hardware platform (e.g., `firmware/euclidean/`) — the stretch goal in the spec §8.

## Documentation

- **[Specification](docs/generative-arp-module.md)** — hardware spec, firmware architecture, BOM, calibration procedure
- **[Decisions](docs/decisions.md)** — tooling, scope, workflow, licensing, versioning
- **[Stories](docs/stories/)** — user-story-driven development backlog

## Toolchain

- **Firmware:** [PlatformIO](https://platformio.org/) with Seeed's platform fork (`platform = https://github.com/Seeed-Studio/platform-seeedboards.git`, `board = seeed_xiao_ra4m1`)
- **Tests:** host-side unit tests via `pio test -e native` for pure-logic modules (scales, arp, generative engine). Hardware abstraction layer is bench-verified.
- **Hardware:** KiCad (project deferred — see decisions)
- **CI:** GitHub Actions runs host tests on push and PR

## Versioning

- **Firmware:** [Semantic Versioning](https://semver.org/), scoped tags per firmware: `arp/v0.1.0`, future `euclidean/v0.1.0`, etc.
- **Hardware:** `Rev N.M` notation, silkscreened on the PCB
- **Docs:** unversioned; history lives in `git log`

See [decisions §12](docs/decisions.md) for full rules.

## Licensing

Three licenses, one per artefact class:

| Area | License | File |
|---|---|---|
| Firmware | MIT | [`LICENSE-firmware`](LICENSE-firmware) |
| Hardware (KiCad, panel artwork) | CERN-OHL-S v2 | [`LICENSE-hardware`](LICENSE-hardware) |
| Documentation | CC-BY-SA 4.0 | [`LICENSE-docs`](LICENSE-docs) |

Chosen to match the surrounding Eurorack DIY ecosystem (Hagiwo, Modulove, WGD) so downstream hackers can remix both sides without license-hunting.
