# firmware/arp

PlatformIO project for the generative arpeggiator firmware running on a Seeed XIAO RA4M1.

## Status

Story 002 (XIAO blinks) — toolchain verification. Current `src/main.cpp` is a minimal blinky toggling `LED_BUILTIN` at 1 Hz to prove the Seeed + PlatformIO toolchain works end-to-end. Subsequent stories layer the actual arp features on top (see [`../../docs/stories/`](../../docs/stories/)).

## Toolchain

- [PlatformIO Core](https://platformio.org/install/cli) 6.x
- Seeed's platform fork: `platform = https://github.com/Seeed-Studio/platform-seeedboards.git`
- Board: `seeed-xiao-ra4m1` (hyphens — matches the platform manifest)
- Framework: Arduino (`ArduinoCore-renesas`)

The Seeed XIAO RA4M1 shares the Renesas RA4M1 MCU with the Arduino Uno R4 Minima, so most of the Arduino core is mature.

## Build

```sh
pio run -d firmware/arp
```

First run downloads the Seeed platform fork, the Renesas toolchain, and `ArduinoCore-renesas` (~hundreds of MB). Subsequent builds are fast.

## Upload

XIAO RA4M1 uploads over native USB using the board's DFU bootloader.

```sh
pio run -d firmware/arp --target upload
```

If the upload hangs or the port isn't found:
- Double-tap the RESET button on the XIAO to force bootloader mode, *then* re-run the upload
- Or hold BOOT while plugging in USB

## Monitor

```sh
pio device monitor -d firmware/arp -b 115200
```

`monitor_speed = 115200` is set in `platformio.ini`; the `-b` flag above is just defensive.

## Tests

No tests yet. Host-side unit tests (`pio test -e native`) land in Story 003 (scale quantiser) — see [decisions §5](../../docs/decisions.md).

## LED

`LED_BUILTIN` on the XIAO RA4M1 is **active-LOW**: `digitalWrite(LED_BUILTIN, LOW)` turns it on, `HIGH` turns it off.
