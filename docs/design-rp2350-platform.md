# RP2350 Platform Design

**Date:** 2026-04-19 (evening)
**Status:** decided, pending implementation
**Supersedes:**
- MCU-specific parts of [`design-pivot-2026-04-19.md`](./design-pivot-2026-04-19.md) (morning same day) — pin budget, software I²C workaround, the D4 ADC gotcha. The UI / form-factor / NeoPixel decisions from that doc carry forward unchanged.
- [`generative-arp-module.md`](./generative-arp-module.md) §2.3 (pin table), §2.5 (DAC output stage), §2.6 (gate output) are re-scoped to RP2350 + PWM. §2.7 (WS2812B) already superseded by morning pivot (onboard NeoPixel).

---

## 1. One-sentence pivot

**Swap MCU from XIAO RA4M1 to XIAO RP2350, replace the DAC V/Oct path with a 10-bit PWM + RC filter + op-amp buffer, and re-position the project as a *generic Eurorack platform* — a fixed-pin board that hosts swappable firmwares (arp first; LFO, quantizer, clock utilities, etc. later), each distributed as an independent `.uf2`.**

---

## 2. Why this pivot

Three forces combined:

1. **The RA4M1's DAC is not the killer feature it appeared to be.** The original MCU-selection rationale was "RA4M1 has a true DAC, which matters for V/Oct accuracy." HAGIWO's MOD2 — a successful, widely-shared Eurorack platform — uses 10-bit PWM + RC + op-amp, publicly acknowledges it's slightly worse than a DAC, and ships anyway. A 10-bit PWM at a few tens of kHz, low-pass filtered, gives ±2¢ V/Oct tracking with the right op-amp and calibration — matching what we got from the RA4M1 DAC on the breadboard (Story 004).

2. **RP2350 is where the ecosystem is.** HAGIWO MOD2 is XIAO RP2350. His firmwares (sample player, kick, snare, hi-hat, FM drum, VCO) are reference implementations we can study, even if we don't run them binary-unchanged. Matching the MCU puts us on the same platform as a growing body of hobbyist work.

3. **RP2350 dissolves the RA4M1 pin-pain we'd been working around.** The RA4M1 had three chip-specific frustrations: D4 silkscreened A4 but not ADC-capable, hardware I²C fixed to D4/D5 (costing D5's ADC), and only 5 ADC pins total. On RP2350 none of those exist: I²C is flexible, PWM is on every GPIO, and the XIAO RP2350's ADC count (3) is sufficient for our current budget.

**Secondary benefits, not driving the decision:**
- Dual core + PIO + ~16× the RAM + ~8× the flash — useful headroom for future firmwares, but we're not designing for hypothetical future requirements
- Bigger community, more tutorials, better Arduino core support long-term
- Onboard NeoPixel can still shine through a panel cutout (same trick as the morning pivot)

---

## 3. What this doc is *not*

- **Not a clone of MOD2.** MOD2 has 3 pots + button + LED + audio-rate output and its firmwares use that topology. Our board has 1 pot + encoder + OLED + control-rate CV output. HAGIWO firmwares are a *reference*, not a drop-in library.
- **Not a fork of MOD2's hardware.** We're borrowing the MCU and PWM output stage philosophy; the rest is our own. "MOD2-inspired" is the right phrase.
- **Not a final schematic.** The pin contract is locked; component values on the PWM output stage and input protection still need breadboard bring-up (§6 and §7).

---

## 4. Pin contract (THE platform artifact)

Every firmware for this board must honor this pin map. This is the hardware contract.

### 4.1 Verified XIAO RP2350 pinout

Confirmed via the `arduino-pico` variant header `variants/seeed_xiao_rp2350/pins_arduino.h` and Seeed wiki. Do not trust pinout images elsewhere on the internet — several disagree with the authoritative source.

| Silkscreen | RP2350 GPIO | ADC? | Default peripheral |
|---|---|---|---|
| D0 / A0 | GP26 | ADC0 | — |
| D1 / A1 | GP27 | ADC1 | — |
| D2 / A2 | GP28 | ADC2 | — |
| D3 | GP5 | no | SPI0 CS |
| D4 | GP6 | no | **Wire1 SDA (I²C1)** |
| D5 | GP7 | no | **Wire1 SCL (I²C1)** |
| D6 | GP0 | no | UART0 TX |
| D7 | GP1 | no | UART0 RX |
| D8 | GP2 | no | SPI0 SCK |
| D9 | GP4 | no | SPI0 MISO |
| D10 | GP3 | no | SPI0 MOSI |

**Board-internal (not on header):**
| GPIO | Function |
|---|---|
| GP22 | Onboard NeoPixel data |
| GP23 | Onboard NeoPixel power enable (drive HIGH) |
| GP25 | User LED (yellow, active-low) |
| GP29 / ADC3 | VBAT/2 (battery sense — not available for external use) |

**Key constraints compared to the XIAO RA4M1:**
- Only **3 ADC pins** (D0/D1/D2), not 5
- PWM on every GPIO (no pin starvation)
- All GPIO support interrupts
- Hardware I²C at D4/D5 is on `Wire1`, not `Wire`

### 4.2 Function assignments for this platform

| XIAO Pin | GPIO | Function | Type | Notes |
|---|---|---|---|---|
| D0 / A0 | GP26 | Tempo pot | ADC in | 100 kΩ linear, wiper → D0, CCW→GND, CW→3V3 |
| D1 / A1 | GP27 | CV in | ADC in | Protected, 0–5 V → 0–3.3 V via divider (spec §2.4.2) |
| D2 / A2 | GP28 | **Reserved** | ADC/digital in | Future 2nd CV in **or** extra button. Route to a PCB pad + silkscreen; don't populate on v1 panel unless a use case firms up |
| D3 | GP5 | Encoder switch | Digital in | Pullup, active-low |
| D4 | GP6 | I²C SDA → OLED | Digital (I²C) | See §4.3 on Wire vs Wire1 |
| D5 | GP7 | I²C SCL → OLED | Digital (I²C) | See §4.3 |
| D6 | GP0 | Encoder phase A | Digital in | Interrupt, pullup |
| D7 | GP1 | Encoder phase B | Digital in | Interrupt, pullup |
| D8 | GP2 | PWM → V/Oct out | Digital (PWM) | Drives RC filter + op-amp stage (§6) |
| D9 | GP4 | Gate in | Digital in | Protected, Schmitt trigger or firmware debounce |
| D10 | GP3 | Gate / trig out | Digital out | NPN level-shift to 5 V (spec §2.6) |
| GP22 | — | Status NeoPixel | Internal | Adafruit_NeoPixel, shines through panel cutout |
| GP25 | — | User LED | Internal | Available for beat indicator or debug |

**Totals:** all 11 exposed pins allocated. 1 is reserved (D2) for a future button or 2nd CV in but unpopulated on the first PCB.

### 4.3 I²C bus routing — important quirk

The default Arduino `Wire` object on XIAO RP2350 maps to I²C0 on GP16/GP17 (back-pad only, not broken out to the header). `Wire1` maps to I²C1 on GP6/GP7 = D4/D5 (header pins).

**However,** `arduino-pico` currently has an open bug on `Wire1` for XIAO RP2350 — see earlephilhower/arduino-pico#2808. Status as of writing: unresolved.

**Mitigation:** remap `Wire` (I²C0) to D4/D5 explicitly at boot:

```cpp
Wire.setSDA(6);   // GP6 = D4
Wire.setSCL(7);   // GP7 = D5
Wire.begin();
```

This works because RP2350 peripherals can be mapped to many pin pairs. Validate during OLED bring-up (Story 010 proposed).

---

## 5. Platform model

**One `.uf2` per firmware personality.** No runtime mode switching, no "mega firmware." The user reflashes to change personality.

- Each firmware is a separate PlatformIO project (or separate `env:` within one): `firmware/arp/`, `firmware/lfo/`, `firmware/quantizer/`, etc.
- Each firmware honors the pin contract in §4.2. Deviating from the contract means a firmware won't work on this board — that's the point.
- Shared pure-logic code (scales, MIDI math, quantization tables) can live in a shared library dir once we have 2+ firmwares. Don't pre-extract — wait for duplication to show the right abstraction.
- Versioning scheme from `decisions.md` §12 holds: `arp/vX.Y.Z` scoped tags, SemVer per firmware.

**Why this model over mode-switching or mega-firmware:**
- Matches HAGIWO MOD2's distribution pattern (one `.uf2` per MOD2 personality)
- Each firmware is independently buildable, versionable, testable
- No shared RAM/flash budget to negotiate
- User experience: drag-and-drop to reflash — cost is low for a hobby DIY Eurorack module

---

## 6. V/Oct output stage (PWM → CV)

### 6.1 Target spec
- Range: C3–C7 (0–4 V), same as spec §2.5.1
- Resolution: at least 10-bit (1024 levels) over 5-octave range = ~1 mV per step — comfortably better than the ±2¢ (~2 mV) tracking target
- Tracking: ±2¢ per octave or better, verified against Plaits / Castor & Pollux
- Settling time: < 1 ms after step change (requirement: gate edge must not precede settled CV)

### 6.2 Topology
```
GP2 (D8) ──┬── R1 ──┬── R2 ──┬── + MCP6002 (or TL072) ── R_out ── CV out jack
 PWM @ N kHz │       │        │   non-inv, gain ~1.27× (3.3 V × 1.27 ≈ 4.2 V headroom)
             C1     C2        │
             │       │        Rf / Rg sets gain
            GND     GND       offset trim TBD
```

- **PWM frequency:** start at 36.6 kHz (HAGIWO's number); adjust during bench test if audible ripple is a problem
- **PWM resolution:** 10-bit native (matches HAGIWO). Higher resolution is possible on RP2350 but costs carrier frequency.
- **LPF:** two-pole RC, cutoff TBD from bench. Starting point: R=3.9 kΩ, C=10 nF each stage (cutoff ~4 kHz) — well above the V/Oct control rate but low enough to kill the PWM carrier. Tune on breadboard.
- **Op-amp:** MCP6002 (on hand, used in Stories 003–005) is the starting choice. TL072 is the HAGIWO default and may track better under load; switch if bench shows MCP6002 struggles. TLV9062 is a modern option worth keeping in mind for a Rev 0.2.
- **Gain:** ~1.27× to lift 3.3 V PWM → 4.2 V with headroom, then firmware calibration maps 0–4 V onto C3–C7. Reuse the calibration approach from Story 004.

### 6.3 What we don't know
- **Exact RC values** — HAGIWO's MOD2 output stage values are behind his Patreon. We'll breadboard-measure what works.
- **Whether the 10-bit/36.6 kHz combo ripple is audible at the VCO** — has to be measured with a scope and then listened to.
- **Whether the same op-amp stage from Story 004 (gain 1.261×, R1=10k/R2=2.7k) transfers cleanly** — probably yes for the op-amp values, but the PWM input needs the RC filter added before the op-amp input.

---

## 7. Input protection (CV in, gate in)

Carry forward from the existing spec with no design change — the protection circuits don't care whether the MCU is RA4M1 or RP2350.

- **CV in (D1):** voltage divider scales Eurorack ±5 V or 0–5 V down to 0–3.3 V. Use the values bench-verified in `spec §2.4.2` (Rev 0.1a): R = 100 kΩ / 220 kΩ divider. Clamp diodes to 3V3 and GND rails. Confirm the RP2350 ADC input impedance is compatible (it is — RP2350 ADC is conventional SAR, same ballpark as RA4M1).
- **Gate in (D9):** protected digital input. Reuse the existing approach from spec §2.4.1 if present, otherwise: series resistor + clamp diodes + Schmitt trigger or firmware debounce.

---

## 8. UI (encoder + OLED + NeoPixel)

Unchanged from the morning design pivot. See [`design-pivot-2026-04-19.md`](./design-pivot-2026-04-19.md) §6 for the UX model (rotate=change value, short-press=next parameter, long-press=context action) and §5 for panel sketch.

**OLED:** 128×32 mono SSD1306, Midas MDOB128032GV-WI or equivalent. Vertical mount in 2HP panel. I²C on D4/D5 per §4.3.

**Encoder:** PEC11 family, mechanical, with push-switch. Phase A/B on D6/D7, switch on D3. Library choice: `RotaryEncoder` by Matthias Hertel is polling-friendly and well-maintained; `Encoder.h` (PJRC) is classic and interrupt-driven. Decide at bring-up.

**NeoPixel (onboard):** GP22 data, GP23 power enable. `Adafruit_NeoPixel` library. Per-firmware colour semantics:
- For arp: e.g. hue = current scale, brightness pulse = step
- For other firmwares: defined by the firmware
- No cross-firmware contract on meaning — just on presence

---

## 9. Form factor

Unchanged from the morning pivot: **2HP target, strict fallback to 4HP** if the panel mockup isn't readable.

- **Jacks (4):** CV out, gate out, CV in, gate in — single column
- **Controls:** tempo pot, encoder (with click), OLED window, reserved button pad (unpopulated on v1 panel unless a use case firms up)
- **Panel approach:** produce a readable mockup (Inkscape or KiCad) *before* cutting a PCB. If it doesn't fit legibly, go 4HP. The 10 mm saved by 2HP isn't worth a bad user interface.

HAGIWO MOD2 is 4HP for comparison; we're attempting 2HP because we have fewer controls and jacks.

---

## 10. Arduino core & PlatformIO

### 10.1 Core
Earle Philhower's **arduino-pico** — <https://github.com/earlephilhower/arduino-pico>. Active, stable for RP2350 as of v5.5.1 (March 2026). Variant `seeed_xiao_rp2350` is upstream.

### 10.2 PlatformIO
Two viable routes:

**Option A — Seeed's platform (matches what we did for RA4M1):**
```ini
[env:seeed-xiao-rp2350]
platform = https://github.com/Seeed-Studio/platform-seeedboards.git
board = seeed-xiao-rp2350
framework = arduino
```

**Option B — Maxgerhardt's arduino-pico platform:**
```ini
[env:seeed_xiao_rp2350]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = seeed_xiao_rp2350
framework = arduino
```

Note the board-id casing differs. **Start with Option A** for continuity with the existing RA4M1 project (same platform publisher, same style). Fall back to B if A is stale.

### 10.3 Flashing
BOOTSEL: hold BOOT, click RESET, release BOOT — standard RP2040/2350 flow. Mounts as USB mass-storage; drag-drop `.uf2`.

---

## 11. What transfers from the RA4M1 work

The existing firmware (Stories 001–008 on main at `99d1624`) is **mostly reusable**.

**Directly portable (pure logic, no Arduino deps):**
- `lib/scales/` — all 6 scales, `scaleFromPot`, quantizer
- `lib/arp/` — Order enum, pattern table, state machine
- `lib/tempo/` — `bpmFromPot` exponential curve, `stepMsFromBpm`
- All 71 GoogleTest cases — should compile and pass unchanged under `env:native`

**Rewrite for RP2350:**
- `src/main.cpp` — pin assignments, DAC setup, `analogWriteResolution` → PWM setup, I²C init
- `platformio.ini` — board, platform, lib deps
- `firmware/arp/README.md` — upload instructions

**New code needed (no RA4M1 equivalent):**
- PWM V/Oct output module (`lib/pwm_out/` or similar) — replaces `dac_out` in spirit
- OLED driver integration (Adafruit_SSD1306 or U8g2)
- Encoder driver integration (RotaryEncoder or Encoder.h)
- NeoPixel feedback (Adafruit_NeoPixel)
- Menu state machine

**Story pacing:** don't rewrite `main.cpp` in one shot. Re-do the story arc as a fresh bring-up on RP2350 — blink, OLED hello, encoder, PWM V/Oct, then port the arp logic back on top. Proposed stories in §13.

---

## 12. Repo identity debt

The repo is named `xiao-ra4m1-arp`. It's now inaccurate on two axes: (a) MCU is no longer RA4M1, (b) it's becoming a platform rather than an arp-specific project.

**Decision:** carry the debt. Don't rename in this session. A future session will:
- Pick a new name (something like `xiao-module` or a brand name)
- Rename the GitHub repo (redirects are automatic)
- Update all `README.md` references
- Keep git history intact — no force pushes

This isn't technically urgent; code still builds, docs still render. Defer until Jay picks the name.

---

## 13. Proposed next stories

Fresh bring-up arc on RP2350, mirroring the RA4M1 arc but on new hardware:

| # | Title | Goal |
|---|---|---|
| 009 | XIAO RP2350 breadboard bring-up | New PlatformIO project (or re-targeted existing), blink `LED_BUILTIN`, serial at 115200, `.uf2` upload works |
| 010 | OLED on Wire0@D4/D5 | Validate I²C workaround (§4.3), print "hello" to SSD1306 |
| 011 | Encoder read-out to OLED | Rotation changes a number on screen, click clears |
| 012 | NeoPixel status indicator | Power-enable GP23 HIGH, solid colour via Adafruit_NeoPixel on GP22 |
| 013 | PWM V/Oct output + calibration | Rebuild Story 004's V/Oct math against PWM. Bench-verify tracking at ±2¢ against Plaits or C&P |
| 014 | Gate out on D10 | Port Story 005's NPN gate stage, verify on scope |
| 015 | CV in on D1 | Port Story 004-era ADC read + scale-in, validate with a known CV source |
| 016 | Arp firmware, first cut | Wire lib/scales + lib/arp + lib/tempo to the new HAL. Encoder-driven scale/order select. Playable through a VCO. |
| 017+ | Second firmware (LFO? Quantizer?) | First firmware personality that proves the platform model is real |

Stories are dependent on each other roughly in order. 009 → 010 → 011 can be near-parallel once the board is up; 013 is the riskiest (PWM V/Oct accuracy) and should happen early.

---

## 14. Open questions (deferred)

- **PWM output stage R/C values** — measure on breadboard during Story 013
- **Op-amp final choice** — MCP6002 vs TL072 vs TLV9062; decide after Story 013 bench data
- **Encoder library** — RotaryEncoder vs Encoder.h; decide at Story 011
- **OLED library** — Adafruit_SSD1306 (Adafruit_GFX-based, bigger) vs U8g2 (lighter, more display support); decide at Story 010
- **Menu state machine shape** — future grill-me before Story 016
- **Whether Wire1 bug is fixed upstream** by the time we touch it — check #2808 status at Story 010
- **NeoPixel colour semantics for arp firmware** — future design session; doesn't block hardware
- **Whether the reserved pin (D2) gets a button or a 2nd CV in** — defer until a use case firms up; leave PCB pad unpopulated on v1

---

## 15. Related documents

- [`design-pivot-2026-04-19.md`](./design-pivot-2026-04-19.md) — morning pivot (encoder + OLED + 2HP + NeoPixel). UI decisions carry forward; MCU-specific content is superseded by this doc.
- [`generative-arp-module.md`](./generative-arp-module.md) — original spec (Rev 0.1a). Pin table (§2.3), DAC stage (§2.5), WS2812B (§2.7) are now out of date and tracked for revision after the new PCB.
- [`decisions.md`](./decisions.md) — process / tooling / licensing. Still current.
- [`calibration.md`](./calibration.md) — V/Oct calibration from Story 004. Math and workflow transfer to PWM; component values will need re-measurement.
- [`bench-log.md`](./bench-log.md) — running bench measurements. Will gain RP2350 entries from Story 013 onward.
