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
