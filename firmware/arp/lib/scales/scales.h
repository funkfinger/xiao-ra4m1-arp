#pragma once

#include <cstdint>

namespace arp {

enum class Scale : uint8_t {
    Major = 0,
    NaturalMinor,
    PentatonicMajor,
    PentatonicMinor,
    Dorian,
    Chromatic,
    Count,
};

// Quantise a MIDI note to the nearest in-scale note.
//
// All scales are rooted at C (MIDI 0, 12, 24, ..., 120). If `midiNote` is
// already in the scale it is returned unchanged. Otherwise the nearest
// in-scale note is returned; equidistant ties are broken by rounding
// down (towards the lower note).
//
// The semantic is "nearest," not "floor." In pentatonic scales the two
// differ — e.g. in C pentatonic minor (0,3,5,7,10), MIDI D (2) maps to
// E♭ (3), not C (0), because E♭ is closer.
//
// `midiNote` is expected to be in [0, 127]. Values outside that range
// are undefined behaviour.
uint8_t quantize(uint8_t midiNote, Scale scale);

// Map a normalised pot position [0.0, 1.0] to a Scale.
//
// The pot range is divided into 6 equal zones, one per scale:
//   [0.000, 0.167) → Major
//   [0.167, 0.333) → NaturalMinor
//   [0.333, 0.500) → PentatonicMajor
//   [0.500, 0.667) → PentatonicMinor
//   [0.667, 0.833) → Dorian
//   [0.833, 1.000] → Chromatic
//
// Hysteresis: `current` is the previously-selected scale. To prevent
// jitter at ADC noise levels, the active scale's zone is extended by
// ±kScaleHysteresis on each side; the pot must cross the extended
// boundary before the scale changes.
//
// `pot` is clamped to [0.0, 1.0]. At power-on, pass Scale::Major (or
// any valid default) as `current`.
Scale scaleFromPot(float pot, Scale current);

// Hysteresis width, expressed as a fraction of the pot range.
// 0.02 ≈ 2% of rotation, well above ADC noise (typically <0.2% on
// 14-bit reads with decent wiring).
constexpr float kScaleHysteresis = 0.02f;

}  // namespace arp
