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

}  // namespace arp
