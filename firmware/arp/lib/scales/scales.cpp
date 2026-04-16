#include "scales.h"

namespace arp {

namespace {

constexpr uint16_t kMaskMajor            = (1u<<0)|(1u<<2)|(1u<<4)|(1u<<5)|(1u<<7)|(1u<<9)|(1u<<11);
constexpr uint16_t kMaskNaturalMinor     = (1u<<0)|(1u<<2)|(1u<<3)|(1u<<5)|(1u<<7)|(1u<<8)|(1u<<10);
constexpr uint16_t kMaskPentatonicMajor  = (1u<<0)|(1u<<2)|(1u<<4)|(1u<<7)|(1u<<9);
constexpr uint16_t kMaskPentatonicMinor  = (1u<<0)|(1u<<3)|(1u<<5)|(1u<<7)|(1u<<10);
constexpr uint16_t kMaskDorian           = (1u<<0)|(1u<<2)|(1u<<3)|(1u<<5)|(1u<<7)|(1u<<9)|(1u<<10);
constexpr uint16_t kMaskChromatic        = 0x0FFFu;

constexpr uint16_t mask_for(Scale s) {
    switch (s) {
        case Scale::Major:            return kMaskMajor;
        case Scale::NaturalMinor:     return kMaskNaturalMinor;
        case Scale::PentatonicMajor:  return kMaskPentatonicMajor;
        case Scale::PentatonicMinor:  return kMaskPentatonicMinor;
        case Scale::Dorian:           return kMaskDorian;
        case Scale::Chromatic:
        default:                      return kMaskChromatic;
    }
}

constexpr bool in_scale_pc(uint8_t pitchClass, uint16_t mask) {
    return ((mask >> pitchClass) & 1u) != 0u;
}

}  // namespace

uint8_t quantize(uint8_t midiNote, Scale scale) {
    const uint16_t mask = mask_for(scale);
    if (in_scale_pc(midiNote % 12u, mask)) {
        return midiNote;
    }
    // Walk outward in semitones. Check the lower candidate first so that
    // ties (equal distance above and below) break downward.
    for (uint8_t dist = 1u; dist < 12u; ++dist) {
        if (midiNote >= dist) {
            const uint8_t lo = static_cast<uint8_t>(midiNote - dist);
            if (in_scale_pc(lo % 12u, mask)) return lo;
        }
        const uint8_t hi = static_cast<uint8_t>(midiNote + dist);
        if (hi <= 127u && in_scale_pc(hi % 12u, mask)) return hi;
    }
    return midiNote;  // unreachable for well-formed scales (root is always in mask)
}

}  // namespace arp
