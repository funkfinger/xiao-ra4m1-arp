#pragma once

#include <cstdint>

namespace arp {

// Arp orders — how the chord tones are sequenced.
// Letters refer to chord indices: A=root, B=3rd, C=5th, D=octave.
enum class Order : uint8_t {
    Up = 0,        // ABCD
    Down,          // DCBA
    UpDown,        // ABCDCB (6 steps, pingpong)
    DownUp,        // DCBAAB (6 steps, inverted pingpong)
    Skip,          // ACBD — inside-out
    Random,        // random chord tone each step
    Count,
};

// Maximum pattern length across all non-random orders.
constexpr uint8_t kMaxPatternLength = 6;

// Chord size: root, 3rd, 5th, octave.
constexpr uint8_t kChordSize = 4;

// Chord intervals (semitones from root). Major triad + octave.
constexpr uint8_t kChordIntervals[kChordSize] = {0, 4, 7, 12};

// Multi-pattern arpeggiator with selectable order.
// Pure logic — no Arduino deps.
//
// Each step returns a MIDI note. For fixed patterns (Up/Down/UpDown/DownUp/Skip)
// advancement walks a pre-defined index sequence. For Random, each advance()
// picks a uniformly random chord tone.
//
// Random uses an internal LCG seeded via `setSeed()`. Deterministic for tests;
// main loop reseeds at boot with `millis()` or similar.

class Arp {
public:
    static constexpr uint8_t kDefaultRoot = 48;  // C3

    explicit Arp(uint8_t root = kDefaultRoot, Order order = Order::Up);

    uint8_t advance();
    void reset();
    uint8_t current() const;

    void setOrder(Order o);
    Order order() const { return order_; }
    uint8_t step() const { return step_; }
    uint8_t patternLength() const;

    // Seed the internal LCG for Random order. Deterministic given seed.
    void setSeed(uint32_t seed) { rngState_ = seed ? seed : 1u; }

private:
    uint8_t nextChordIndex() const;  // current chord-tone index at step_

    uint8_t root_;
    Order order_;
    uint8_t step_;
    uint8_t randomIndex_;   // cached for Random order (so current() is stable between advances)
    uint32_t rngState_;
};

// Map a normalised pot position [0.0, 1.0] to an Order.
// Pot range divided into 6 equal zones; ±2% hysteresis at boundaries to
// prevent ADC jitter (same approach as scaleFromPot).
Order orderFromPot(float pot, Order current);

constexpr float kOrderHysteresis = 0.02f;

}  // namespace arp
