# V/Oct Calibration — Story 004

## Op-Amp Circuit: MCP6002 Non-Inverting Amplifier

**Gain:** 1.27× (1 + R2/R1 = 1 + 2.7k/10k)
**Output range:** 0–4.0V (4 octaves, C3–C7)

> **Note:** The original spec §2.5.2 stated R2=12.1k (gain 2.21×) but this was a calculation error — 3.3V × 2.21 = 7.3V, not 4V. The correct gain for 0–3.3V → 0–4.0V is 1.22×, achieved with R2=2.7kΩ.

### Parts

| Ref | Value | Source |
|---|---|---|
| U1 | MCP6002-I/P DIP-8 | Cabinet 3, Bin 32, Section C |
| R1 | 10kΩ axial | Cabinet 2, Bin 2 |
| R2 | 2.7kΩ axial | Cabinet 2, Bin 1 area |
| R4 | 1kΩ axial (output series) | Cabinet 2, Bin 1 |
| C1 | 100nF ceramic (decoupling) | Cabinet 2 |

### Wiring Table

| From | To | Notes |
|---|---|---|
| XIAO 5V | MCP6002 pin 8 (VDD) | Power |
| XIAO GND | MCP6002 pin 4 (VSS) | Power |
| C1 leg 1 | MCP6002 pin 8 (VDD) | Decoupling — place close to chip |
| C1 leg 2 | MCP6002 pin 4 (VSS) | Decoupling |
| XIAO D0/A0 | MCP6002 pin 3 (IN1+) | DAC output to op-amp input |
| R1 leg 1 | MCP6002 pin 2 (IN1-) | Gain-setting — feedback node |
| R1 leg 2 | GND | |
| R2 leg 1 | MCP6002 pin 2 (IN1-) | Gain-setting — feedback node (same row as R1 leg 1) |
| R2 leg 2 | MCP6002 pin 1 (OUT1) | Feedback from output |
| R4 leg 1 | MCP6002 pin 1 (OUT1) | Output series resistor |
| R4 leg 2 | Wire to VCO V/Oct input | This is the module output |
| MCP6002 pin 5 (IN2+) | GND | Unused amp — tie input low |
| MCP6002 pin 6 (IN2-) | MCP6002 pin 7 (OUT2) | Unused amp — unity-gain follower |

### MCP6002 DIP-8 Pinout Reference

```
         ┌────U────┐
  OUT1  1│         │8  VDD (+5V)
  IN1- 2│ MCP6002 │7  OUT2
  IN1+ 3│         │6  IN2-
  VSS  4│         │5  IN2+
         └─────────┘
```

### Target Voltage Table

Assumes gain = 1.27× (R2=2.7k, R1=10k). Actual gain will be measured and recorded below.

| Note | MIDI | Target Vout | DAC Count | DAC Voltage |
|---|---|---|---|---|
| C3 | 48 | 0.000 V | 0 | 0.000 V |
| C4 | 60 | 1.000 V | 977 | 0.787 V |
| C5 | 72 | 2.000 V | 1954 | 1.575 V |
| C6 | 84 | 3.000 V | 2931 | 2.362 V |
| C7 | 96 | 4.000 V | 3908 | 3.149 V |

### Measurements

*(Fill in after bench verification)*

| Note | MIDI | Target Vout | Measured (nominal) | Error | Measured (gain-corrected) |
|---|---|---|---|---|---|
| C3 | 48 | 0.000 V | 0.004 V | +4 mV | ~0 mV (offset) |
| C4 | 60 | 1.000 V | 0.993 V | -7 mV | ±2 mV |
| C5 | 72 | 2.000 V | 1.986 V | -14 mV | ±2 mV |
| C6 | 84 | 3.000 V | 2.980 V | -20 mV | ±2 mV |
| C7 | 96 | 4.000 V | 3.975 V | -25 mV | ±2 mV |

**Scope screenshot:** [`requirements/DS1Z_QuickPrint3.png`](../requirements/DS1Z_QuickPrint3.png) — clean staircase, 5 levels (0–4V), 2s/step, 1V/div.

**Measured gain:** 1.261 (nominal 1.27; 5% resistor tolerance)
**Offset at C3:** +4 mV (op-amp input offset voltage, ~5¢)
**Firmware GAIN constant:** updated from 1.27 to 1.261; all voltages within ±2 mV after correction
**Date:** 2026-04-16
**Pitch test:** pending (patch to VCO + tuner app, target ±5¢)
