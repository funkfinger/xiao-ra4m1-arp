# hardware

KiCad project for the module's schematic, PCB, and panel artwork.

**Deliberately empty pending breadboard validation of the signal chain** — see [`../docs/decisions.md`](../docs/decisions.md) §4. The plan:

1. [Story 002](../docs/stories/002-xiao-blinks.md) — firmware toolchain works
2. [Story 003](../docs/stories/003-dac-ramp-and-quantiser.md) — DAC emits clean linear ramp on the bench
3. [Story 004](../docs/stories/004-voct-tracks-octaves.md) — op-amp breadboard, measured gain recorded in `docs/calibration.md`
4. **Then** KiCad — schematic capture uses measured component values, not calculated ones

Hardware revisions use `Rev N.M` notation, silkscreened on the board (see decisions §12). Rev 0.1 is the first fabricated board after breadboard validation; Rev 1.0 is the first publicly-documented release.

Spec lives at [`../docs/generative-arp-module.md`](../docs/generative-arp-module.md).
