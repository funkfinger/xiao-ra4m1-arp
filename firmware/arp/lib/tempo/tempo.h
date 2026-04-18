#pragma once

#include <cstdint>

namespace arp {

// Tempo pot → BPM mapping.
// Exponential curve: each third of the pot rotation doubles the BPM.
//   pot = 0.0  →  40  BPM
//   pot = 0.5  → ~113 BPM
//   pot = 1.0  → 300  BPM
//
// Host-testable: takes a normalised [0.0, 1.0] float, returns integer BPM.
// The ADC read + smoothing live in the main loop / HAL layer.

namespace tempo {

constexpr int kMinBpm = 40;
constexpr int kMaxBpm = 300;

// Map normalised pot position to BPM using exponential curve.
// `pot` is clamped to [0.0, 1.0].
int bpmFromPot(float pot);

// Inverse: given BPM, return period of one step in milliseconds.
// Guards against division by zero by clamping bpm to [kMinBpm, kMaxBpm].
unsigned long stepMsFromBpm(int bpm);

}  // namespace tempo
}  // namespace arp
