#pragma once

#include <cstdint>

namespace arp {

struct ArpState {
    uint8_t step;
    uint8_t numNotes;
};

// Minimal up-arpeggiator for v0.1.
// Cycles through a fixed chord (root, major 3rd, 5th, octave) in
// ascending order, wrapping at the top. Pure logic — no Arduino deps.

class Arp {
public:
    static constexpr uint8_t kDefaultRoot = 48;  // C3
    static constexpr uint8_t kChordIntervals[] = {0, 4, 7, 12};
    static constexpr uint8_t kChordSize = 4;

    explicit Arp(uint8_t root = kDefaultRoot);

    uint8_t advance();
    void reset();
    uint8_t current() const;
    uint8_t step() const { return state_.step; }
    uint8_t numNotes() const { return kChordSize; }

private:
    uint8_t root_;
    ArpState state_;
};

}  // namespace arp
