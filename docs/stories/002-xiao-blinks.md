# Story 002: XIAO Blinks

**As a** developer
**I want** the PlatformIO toolchain to build and flash a minimal firmware to the XIAO RA4M1
**So that** I know the end-to-end toolchain (install, board def, compile, upload, run) works before I write anything non-trivial

## Acceptance Criteria
- [x] `firmware/arp/platformio.ini` references Seeed's platform fork and `board = seeed-xiao-ra4m1`
- [x] `firmware/arp/src/main.cpp` toggles the onboard LED at ~1 Hz using Arduino `setup()` / `loop()`
- [x] `pio run -d firmware/arp` compiles cleanly
- [x] `pio run -d firmware/arp --target upload` flashes a connected XIAO and the LED visibly blinks *(bench verified 2026-04-15 — DFU upload in 15s, 33464 bytes, LED blinks at 1 Hz)*
- [x] CI runs `pio run -d firmware/arp` (compile-check only — no upload) and stays green *(green locally in 35s; CI verification on PR)*
- [x] Calibration, tests, and libraries are not added yet — this PR is strictly "toolchain works"

## Notes
- If the Seeed platform fork has any quirks (e.g., manual installation, specific PlatformIO version), capture them in `firmware/arp/README.md`.
- Upload method: DFU / USB bootloader (double-tap reset to enter bootloader on XIAO RA4M1). Document in firmware README.
- If PlatformIO hits a wall, fall back to `arduino-cli` (see decisions.md §2). Document whichever tool path actually worked.

## Depends on
- Story 001

## Status
done (PR #1, bench verified 2026-04-15)

Local build: 35s on macOS, 33.4 KB flash (12.8%) / 2.5 KB RAM (7.8%). CI build: 44s. DFU upload: 15s. Onboard LED blinks at 1 Hz on the physical XIAO RA4M1. End-to-end toolchain (install → compile → flash → run) confirmed working.

*Known quirk:* first attempt of `--target upload` may fail during a PlatformIO post-install script (`package-postinstall.py` exit 127); a retry succeeds. Harmless.
