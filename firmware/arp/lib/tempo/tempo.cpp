#include "tempo.h"

#include <cmath>

namespace arp {
namespace tempo {

int bpmFromPot(float pot) {
    if (pot < 0.0f) pot = 0.0f;
    if (pot > 1.0f) pot = 1.0f;
    // exponent such that 2^exponent = kMaxBpm / kMinBpm = 7.5
    // exponent ≈ log2(7.5) ≈ 2.907
    constexpr float kExponent = 2.9068906f;
    float bpm = static_cast<float>(kMinBpm) * std::pow(2.0f, pot * kExponent);
    int result = static_cast<int>(bpm + 0.5f);
    if (result < kMinBpm) result = kMinBpm;
    if (result > kMaxBpm) result = kMaxBpm;
    return result;
}

unsigned long stepMsFromBpm(int bpm) {
    if (bpm < kMinBpm) bpm = kMinBpm;
    if (bpm > kMaxBpm) bpm = kMaxBpm;
    return static_cast<unsigned long>(60000.0f / bpm + 0.5f);
}

}  // namespace tempo
}  // namespace arp
