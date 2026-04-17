#include "arp.h"

namespace arp {

constexpr uint8_t Arp::kChordIntervals[];

Arp::Arp(uint8_t root) : root_(root), state_{0, kChordSize} {}

uint8_t Arp::advance() {
    state_.step = (state_.step + 1) % kChordSize;
    return current();
}

void Arp::reset() {
    state_.step = 0;
}

uint8_t Arp::current() const {
    return root_ + kChordIntervals[state_.step];
}

}  // namespace arp
