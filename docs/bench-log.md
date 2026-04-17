# Bench Log

Measurements from hardware bring-up, ordered chronologically. Referenced by stories and calibration notes.

---

## 2026-04-15 — DAC Ramp (Story 003)

**Setup:** XIAO RA4M1 on breadboard, USB-powered. Rigol DS1054Z CH1 probing pin D0/A0 (DAC0). No op-amp — raw DAC output.

**Firmware:** `src/main.cpp` writes a 0→4095 linear ramp (256 steps, `analogWrite` at 12-bit resolution) at ~10 Hz target.

**Scope screenshot:** [`requirements/DS1Z_QuickPrint1.png`](../requirements/DS1Z_QuickPrint1.png)

| Parameter | Measured | Expected | Notes |
|---|---|---|---|
| Full-scale voltage | ~3.3 V | 3.3 V | Matches RA4M1 DAC Vref (AVCC) |
| Ramp frequency | 8.56 Hz | ~10 Hz | Overhead from `delayMicroseconds()` + loop; acceptable |
| Linearity | Clean diagonal, no visible steps | Linear | At 20 ms/div, individual DAC steps are below scope resolution |
| Glitches | None visible | None | No missing codes or spurious spikes |
| Reset edge | Sharp, clean | Sharp | Capacitive loading negligible at this impedance |

**Scope settings:** 1.00 V/div, 20.0 ms/div, 25 MSa/s, rising edge trigger.

**Verdict:** DAC0 confirmed working. Linear output, full 12-bit range, 0–3.3 V. Ready for op-amp scaling stage (Story 004).

---

## 2026-04-16 — Arpeggiator v0.1 (Story 005)

**Setup:** XIAO RA4M1 → MCP6002 (gain 1.261, R4 bypassed) → Plaits V/Oct. Gate via BC548C NPN → envelope trigger. DS1054Z CH1=V/Oct, CH2=Gate.

**Firmware:** `src/main.cpp` integrates `lib/arp/` (up-pattern, C3-E3-G3-C4) + `lib/scales/` (major quantiser) + calibrated DAC output. 120 BPM internal clock, 50% gate duty.

**Scope screenshots:**
- V/Oct staircase: [`requirements/DS1Z_QuickPrint4.png`](../requirements/DS1Z_QuickPrint4.png)
- V/Oct + Gate dual-channel: [`requirements/DS1Z_QuickPrint5.png`](../requirements/DS1Z_QuickPrint5.png)

**Audio recording:** [`requirements/arp.m4a`](../requirements/arp.m4a) — phone mic capture of Plaits output.

| Parameter | Observed |
|---|---|
| Arp pattern | C3 → E3 → G3 → C4, repeating |
| Tempo | 120 BPM (500ms/step) |
| Gate duty | 50% (250ms on / 250ms off) |
| V/Oct levels | 4-step staircase, 0V → 0.33V → 0.58V → 1.0V |
| Gate polarity | Corrected for NPN inversion (D6 LOW = gate on) |

**Verdict:** v0.1 — module makes music. Audible 4-note up-arpeggio, clean gate, calibrated V/Oct.
