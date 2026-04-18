#include "arp.h"

namespace arp {

// Fixed patterns as chord-tone index sequences (A=0, B=1, C=2, D=3).
namespace {

constexpr uint8_t kUpSteps[]     = {0, 1, 2, 3};
constexpr uint8_t kDownSteps[]   = {3, 2, 1, 0};
constexpr uint8_t kUpDownSteps[] = {0, 1, 2, 3, 2, 1};
constexpr uint8_t kDownUpSteps[] = {3, 2, 1, 0, 1, 2};
constexpr uint8_t kSkipSteps[]   = {0, 2, 1, 3};  // ACBD inside-out

struct Pattern {
    const uint8_t* indices;
    uint8_t length;
};

constexpr Pattern kPatterns[] = {
    {kUpSteps,     4},   // Order::Up
    {kDownSteps,   4},   // Order::Down
    {kUpDownSteps, 6},   // Order::UpDown
    {kDownUpSteps, 6},   // Order::DownUp
    {kSkipSteps,   4},   // Order::Skip
    // Random handled specially — no pattern array
};

// Simple LCG — deterministic, sufficient for generative noise.
// Constants from Numerical Recipes.
uint32_t lcgNext(uint32_t state) {
    return state * 1664525u + 1013904223u;
}

}  // namespace

Arp::Arp(uint8_t root, Order order)
    : root_(root), order_(order), step_(0), randomIndex_(0), rngState_(1u) {}

uint8_t Arp::nextChordIndex() const {
    if (order_ == Order::Random) {
        return randomIndex_;
    }
    const uint8_t idx = static_cast<uint8_t>(order_);
    const Pattern& p = kPatterns[idx];
    return p.indices[step_ % p.length];
}

uint8_t Arp::patternLength() const {
    if (order_ == Order::Random) return 1;  // Random "cycle" is 1 step
    return kPatterns[static_cast<uint8_t>(order_)].length;
}

uint8_t Arp::advance() {
    if (order_ == Order::Random) {
        rngState_ = lcgNext(rngState_);
        randomIndex_ = static_cast<uint8_t>((rngState_ >> 16) % kChordSize);
    } else {
        const uint8_t len = patternLength();
        step_ = (step_ + 1) % len;
    }
    return current();
}

void Arp::reset() {
    step_ = 0;
    // Don't reset RNG state — reset() is for pattern position, not generator.
}

uint8_t Arp::current() const {
    const uint8_t chordIdx = nextChordIndex();
    return root_ + kChordIntervals[chordIdx];
}

void Arp::setOrder(Order o) {
    if (o == order_) return;
    order_ = o;
    step_ = 0;  // restart the new pattern from step 0
    // For Random, randomIndex_ is stale but will be refreshed on next advance().
    // current() will return the last-cached random tone until then — acceptable.
}

Order orderFromPot(float pot, Order current) {
    if (pot < 0.0f) pot = 0.0f;
    if (pot > 1.0f) pot = 1.0f;

    constexpr int   kNumOrders = static_cast<int>(Order::Count);
    constexpr float kZoneWidth = 1.0f / kNumOrders;

    const int currentIdx = static_cast<int>(current);
    const float nominalLower = currentIdx * kZoneWidth;
    const float nominalUpper = (currentIdx + 1) * kZoneWidth;
    const float extendedLower = nominalLower - kOrderHysteresis;
    const float extendedUpper = nominalUpper + kOrderHysteresis;

    if (pot >= extendedLower && pot <= extendedUpper) {
        return current;
    }

    int newIdx = static_cast<int>(pot / kZoneWidth);
    if (newIdx < 0) newIdx = 0;
    if (newIdx >= kNumOrders) newIdx = kNumOrders - 1;
    return static_cast<Order>(newIdx);
}

}  // namespace arp
