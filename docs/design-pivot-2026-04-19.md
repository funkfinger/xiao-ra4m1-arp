# Design Pivot — Encoder + OLED UI, 2HP form factor

**Date:** 2026-04-19
**Status:** decided but not yet implemented
**Supersedes:** spec §2.1 (form factor), §2.3 (pin table), §2.7 (LED), §2.8 (pots), §3.1 (layout)

This document captures an architectural pivot in the UI and form factor of the module. The decisions here are not yet reflected in firmware; Stories 007 and 008 (pot-based scale and order selection) remain in main as of `99d1624` and will be superseded when the pivot is implemented.

---

## 1. The pivot in one sentence

**Replace the 3-pot + button UI with: 1 tempo pot + 1 rotary encoder (click + rotate) + 1 OLED display, shrinking the module from 4HP to 2HP and freeing analog pins for more CV modulation inputs.**

---

## 2. What changed and why

### 2.1 From 4HP to 2HP
- **Why:** tighter use of rack space; module becomes a focused tool rather than a swiss army knife.
- **Proof of possible:** Hagiwo MOD2 (also XIAO-based) fits 3 pots + button + LED + 4 jacks in 2HP. We're using the same physical template.
- **Implication:** component bodies (pots, encoders) are wider than 2HP but mount *behind* the panel — only bushings/cutouts need to fit 10.16mm width.

### 2.2 From 3 pots to 1 pot + encoder + display
- **Why:** discrete-value selection (6 scales, 6 arp orders) feels imprecise on a 300° pot — each option gets only ~50°. An encoder gives infinite rotation with tactile detents. A display lets you see what you're changing.
- **Tempo stays a pot** because it's a continuous, live-tweak parameter you want to grab without menus.
- **Everything else moves into the encoder menu:** scale, arp order, chaos level, chord root, potentially more.
- **User feedback from Story 007:** pot-based scale select works but feels fidgety. Jay flagged button-cycle or encoder as preferable — this pivot resolves it.

### 2.3 Adding an OLED
- **Part:** Midas MDOB128032GV-WI, 128×32 monochrome OLED, I²C, SSD1306-compatible.
- **Dimensions:** PCB 12.0 × 35.8mm; active area 22.38 × 5.58mm.
- **Orientation:** mounted *vertically* in the panel — active strip becomes 5.58mm wide × 22.38mm tall. Fits 2HP comfortably.
- **Role:** shows currently-selected parameter name + value + simple status (e.g., "SCALE: Pentatonic Minor").

### 2.4 Freeing analog pins for CV modulation
- **Why:** in Eurorack, every ADC pin can become a CV input = another patch point = more sonic flexibility.
- **Before (3 pots + 1 CV):** 4 analog inputs consumed, all dedicated.
- **After (1 pot + 3 CVs):** 4 analog inputs, but 3 are patchable CVs.
- **The OLED I²C bus is moved off the hardware I²C pins (D4/D5) to preserve D5's ADC capability.**

---

## 3. XIAO RA4M1 pin-capability audit (2026-04-19)

Verified against Seeed wiki + Arduino core + Seeed forum. **Previous spec §2.3 had errors.**

### 3.1 ADC-capable pins
**Only 5 pins support `analogRead()`:** D0, D1, D2, D3, D5.

### 3.2 Documentation gotcha
- **D4 is silkscreened "A4"** on the XIAO RA4M1 pinout diagram — but `analogRead(4)` returns PWM-like noise, not a real ADC reading.
- D4 maps internally to a port (P206 per one source, P101 per another) that is **not** connected to the RA4M1's ADC peripheral.
- Treat D4 as digital-only. Don't trust the A4 label.
- Reference: [Seeed forum](https://forum.seeedstudio.com/t/xiao-ra4m1-analog-a4-and-a5-pins-unexplained-behavior/283077).

### 3.3 Hardware I²C
- Hardware I²C peripheral fixed to **D4 (SDA) and D5 (SCL)**.
- **If we use hardware I²C, D5 loses its ADC capability** (the I²C uses it as a clock line).
- Software (bit-banged) I²C can run on any two digital pins at ~50kHz (vs ~400kHz hardware).

### 3.4 Non-ADC pins (fully digital)
**6 pins:** D4, D6, D7, D8, D9, D10.

### 3.5 Onboard LEDs
- **User LED** on port P011, exposed as `LED_BUILTIN`. Active-LOW. Monochrome.
- **RGB NeoPixel** on port P112 with enable on P500, exposed as `RGB_BUILTIN`. **Does NOT consume any of D0–D10.** Usable with `Adafruit_NeoPixel` library.

---

## 4. Locked-in pin budget (Option C)

| XIAO Pin | Function | Notes |
|---|---|---|
| D0 / A0 | DAC → V/Oct out | Permanent — only DAC on this MCU |
| D1 / A1 | CV in #1 | ADC. Protected input, scaled 0–5V → 0–3.3V via divider |
| D2 / A2 | Tempo pot | ADC. 100k pot wired CCW=GND, wiper→D2, CW=3V3 |
| D3 / A3 | CV in #2 | ADC. Chaos / modulation CV |
| D4 | Free — spare digital | Could be extra button or LED |
| D5 / A5 | CV in #3 | ADC — **preserved** by moving I²C off D5 |
| D6 | Gate out | Via NPN level-shift to 5V (existing circuit) |
| D7 | Clock in | Protected digital input |
| D8 | **Software I²C SDA** | OLED + I²C encoder share bus |
| D9 | **Software I²C SCL** | OLED + I²C encoder share bus |
| D10 | Free — spare digital | Could be extra button, tap-tempo, etc. |
| P011 (internal) | Beat indicator LED | `LED_BUILTIN` — existing firmware usage |
| P112 (internal) | RGB NeoPixel feedback | `RGB_BUILTIN` — menu state colour |

**Totals:**
- 4 analog inputs: 1 pot + 3 CVs
- 2 digital ins: clock + 1 spare
- 1 digital out: gate + 1 spare digital
- 2 internal feedback LEDs + OLED via I²C
- Encoder on I²C (shared bus), no dedicated pins

**Why software I²C:** hardware I²C forces D5 to be SCL and costs us the D5 ADC. Software I²C on D8/D9 (both non-ADC) keeps all 4 ADC pins for analog inputs. Trade: ~20ms full OLED refresh (software) vs ~1ms (hardware). Fine for control-rate UI updates.

---

## 5. Control surface (2HP panel layout, top to bottom)

```
┌──────┐
│  [O] │  OLED — 5.58mm wide × 22.38mm tall active area
│  [O] │  vertical orientation, shows parameter + value
│  [O] │
├──────┤
│  [ ] │  Tempo pot (9mm bushing, 100kΩ linear)
├──────┤
│  [ ] │  Rotary encoder (PEC11 family, 13mm body behind panel)
├──────┤
│ [J][J]│ Clock in | V/Oct out
│ [J][J]│ CV in (#1 or mux?) | Gate out
└──────┘
```

Additional CV inputs (D3, D5) need jacks too — may require moving to 3HP or adding a second row of jacks. **Panel jack count is the next unresolved question.**

---

## 6. UX model (encoder + OLED menu)

### 6.1 Parameter set
Accessible via encoder menu:
- **Scale** (6 choices: Major, Natural Minor, Pentatonic Major/Minor, Dorian, Chromatic)
- **Arp order** (6 choices: Up, Down, UpDown, DownUp, Skip, Random)
- **Chord root** (MIDI 0–127 or restricted to one octave)
- **Chaos amount** (future)
- Future: more as needed

### 6.2 Interaction pattern
- **Rotate encoder:** change current parameter's value
- **Click (short press):** cycle to next parameter
- **Double-click or long-press:** cycle back / enter settings / reset
- **OLED:** always shows active parameter and its value
- **RGB NeoPixel:** colour encodes which parameter is active (green=scale, blue=order, red=chaos, etc.)
- **Tempo pot:** remains always-live, independent of menu state

### 6.3 State machine sketch (TBD in future grill-me)
```
MenuState
  ├── current_parameter: enum (Scale, Order, Root, Chaos, ...)
  ├── encoder_delta → update current_parameter's value
  ├── encoder_click → advance current_parameter (wraps)
  └── encoder_long_press → context action (reset? settings?)
```

---

## 7. Components on hand (inventory-checked 2026-04-19)

| Component | Location | Qty | Status |
|---|---|---|---|
| XIAO RA4M1 | breadboard | 1 | wired, working |
| MCP6002-I/P DIP-8 | Cabinet 3 Bin 32 Section C | ~20 | working on breadboard |
| BC548C NPN TO-92 | Cabinet 2 Bin 26 | 20 | working on breadboard |
| 10kΩ resistor | Cabinet 2 Bin 2 | 20+ | in use |
| 2.7kΩ resistor | breadboard | 1 | in use (op-amp gain) |
| 100Ω resistor | (need to locate) | ? | not yet substituted for R4 |
| 74HC4051 8-ch analog mux | Cabinet 3 Bin 34 Section D | 15 | **available if analog-count grows** |
| PEC11 encoder | Jay has datasheet — confirm inventory | ? | **needs checking** |
| OLED MDOB128032GV-WI | Jay mentioned this + "different one that fits width" | ? | **needs checking** |
| I²C rotary encoder (e.g. Adafruit 4991 Seesaw) | — | 0 | **need to source** |

---

## 8. Known spec errors this pivot fixes or supersedes

- **§2.1** form factor: 4HP → 2HP (pending Jay's final call — may stay 4HP if panel layout gets tight)
- **§2.3** pin table: replaced by §4 of this doc
- **§2.4.2** CV input: restructured — no pot sharing, CV input has its own ADC
- **§2.7** WS2812B RGB LED: replaced by onboard XIAO NeoPixel (RGB_BUILTIN) — no external LED needed
- **§2.8** pots: reduced from 3 to 1 (tempo only)
- **§3.1** panel layout: replaced by §5 of this doc
- **§4.2** generative engine: controls move from pots to encoder menu; concept unchanged

---

## 9. Next steps (when we pick this up again)

1. **Confirm part availability:** encoder (PEC11 or I²C version), 128×32 OLED, any other missing parts.
2. **Grill-me follow-ups:**
   - UX state machine: exact behaviour of click vs long-press vs double-click
   - NeoPixel colour semantics: which parameter = which colour?
   - Jack count: 2HP can fit 4 jacks (2×2) but 3 CV inputs + clock in + V/Oct out + gate out = 6 jacks. Trade-off: drop a CV input, or go to 3HP, or stack jacks.
   - Software I²C library choice: `SoftwareWire`, `SlowSoftI2CMaster`, or hand-rolled?
   - Encoder library: `Encoder.h` (classic, interrupt-driven) vs `RotaryEncoder.h` (polling-friendly)
3. **Stories to write:** 
   - Story 009: OLED + encoder brought up on breadboard (no arp integration yet, just "encoder rotation prints to OLED")
   - Story 010: Replace Story 007/008 pot-based UI with encoder menu — scale + order as menu items
   - Story 011: Add CV inputs on D3 and (optionally) D5
   - Story N: chaos integration (currently deferred)

4. **Whether to keep Stories 007 and 008 in main:** they work as-is, but the pots will be repurposed. Options:
   - Revert them before starting encoder work (cleaner history, but throws away working code)
   - Leave them in; replace main.cpp in the encoder story (history shows evolution)
   - Keep the `scaleFromPot` / `orderFromPot` functions (they're pure logic, reusable for CV-in → parameter mapping in the future)

**Recommendation:** leave Stories 007/008 in. The pot-read functions become CV-read functions in Story 011 anyway — same math, different source. No wasted work.

---

## 10. Open questions deliberately left for later

- **Jack count on 2HP.** If we want 3 CV ins + clock in + V/Oct + gate = 6 jacks, 2HP is tight. Decide between: 3HP, stacked jacks, drop CV ins.
- **Panel height budget.** OLED + pot + encoder + 4+ jacks vertically stacked ~ needs 60–90mm. Fits Eurorack 3U (~110mm interior) but no generous margins.
- **Encoder sourcing.** PEC11 is common and cheap; I²C Seesaw encoders ($6+) add flexibility but also BOM.
- **Software I²C reliability.** Should work at 50kHz on a Cortex-M4 at 48MHz — but worth a bench smoke test before committing.
- **Chaos design.** Still deferred per earlier session note.

---

## 11. Related memory notes (for future sessions)

See `~/.claude/projects/-Users-jayw-Git-xiao-ra4m1-arp/memory/`:
- `rotary_encoder_reconsideration.md` — original encoder discussion
- `scale_pot_vs_button_reconsideration.md` — earlier UX note (now superseded)
- `arp_orders_fingerpicking_idea.md` — guitar-pattern arp orders
- `xiao_ra4m1_pinout.md` — pin numbering reference
