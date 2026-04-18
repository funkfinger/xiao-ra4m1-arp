#include <gtest/gtest.h>

#include <cmath>

#include "tempo.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace {

using arp::tempo::bpmFromPot;
using arp::tempo::stepMsFromBpm;
using arp::tempo::kMinBpm;
using arp::tempo::kMaxBpm;

// ──────────────────────────────────────────────────────────────────
// Endpoint behaviour — pot extremes map to BPM extremes.
// ──────────────────────────────────────────────────────────────────

TEST(Tempo, PotZeroIsMinBpm) {
    EXPECT_EQ(bpmFromPot(0.0f), 40);
}

TEST(Tempo, PotOneIsMaxBpm) {
    EXPECT_EQ(bpmFromPot(1.0f), 300);
}

TEST(Tempo, ConstantsMatchRequirement) {
    EXPECT_EQ(kMinBpm, 40);
    EXPECT_EQ(kMaxBpm, 300);
}

// ──────────────────────────────────────────────────────────────────
// Clamping — out-of-range inputs clip to valid BPM.
// ──────────────────────────────────────────────────────────────────

TEST(Tempo, NegativePotClampsToMin) {
    EXPECT_EQ(bpmFromPot(-0.5f), kMinBpm);
    EXPECT_EQ(bpmFromPot(-1000.0f), kMinBpm);
}

TEST(Tempo, AbovePotOneClampsToMax) {
    EXPECT_EQ(bpmFromPot(1.5f), kMaxBpm);
    EXPECT_EQ(bpmFromPot(1000.0f), kMaxBpm);
}

// ──────────────────────────────────────────────────────────────────
// Exponential curve — equal pot intervals produce equal musical ratios.
// ──────────────────────────────────────────────────────────────────

TEST(Tempo, EqualPotIntervalsDoubleBpm) {
    // With kExponent = log2(7.5) ≈ 2.907, the curve doubles every 1/2.907
    // ≈ 0.344 of pot rotation. Verify two consecutive doublings inside
    // the pot range (third iteration would exceed 1.0 and clamp).
    constexpr float doublingStep = 1.0f / 2.9068906f;
    int bpm0 = bpmFromPot(0.0f);
    int bpm1 = bpmFromPot(doublingStep);
    int bpm2 = bpmFromPot(2 * doublingStep);

    EXPECT_NEAR(static_cast<float>(bpm1) / bpm0, 2.0f, 0.05f);
    EXPECT_NEAR(static_cast<float>(bpm2) / bpm1, 2.0f, 0.05f);
}

TEST(Tempo, CurveIsMonotonicallyIncreasing) {
    int prev = bpmFromPot(0.0f);
    for (float p = 0.01f; p <= 1.0f; p += 0.01f) {
        int cur = bpmFromPot(p);
        EXPECT_GE(cur, prev) << "non-monotonic at pot=" << p;
        prev = cur;
    }
}

// ──────────────────────────────────────────────────────────────────
// Midpoint sanity check — at pot = 0.5, BPM should be the geometric mean
// of min and max (≈ sqrt(40 * 300) ≈ 109.5).
// ──────────────────────────────────────────────────────────────────

TEST(Tempo, MidpointIsGeometricMean) {
    int bpm = bpmFromPot(0.5f);
    float expected = std::sqrt(static_cast<float>(kMinBpm) * kMaxBpm);
    EXPECT_NEAR(bpm, static_cast<int>(expected + 0.5f), 2);
}

// ──────────────────────────────────────────────────────────────────
// stepMsFromBpm — inverse mapping for main-loop timing.
// ──────────────────────────────────────────────────────────────────

TEST(Tempo, StepMsAt120BpmIs500) {
    EXPECT_EQ(stepMsFromBpm(120), 500u);
}

TEST(Tempo, StepMsAt60BpmIs1000) {
    EXPECT_EQ(stepMsFromBpm(60), 1000u);
}

TEST(Tempo, StepMsAtMinBpm) {
    // 40 BPM → 1500ms
    EXPECT_EQ(stepMsFromBpm(kMinBpm), 1500u);
}

TEST(Tempo, StepMsAtMaxBpm) {
    // 300 BPM → 200ms
    EXPECT_EQ(stepMsFromBpm(kMaxBpm), 200u);
}

TEST(Tempo, StepMsClampsZero) {
    EXPECT_EQ(stepMsFromBpm(0), stepMsFromBpm(kMinBpm));
}

TEST(Tempo, StepMsClampsNegative) {
    EXPECT_EQ(stepMsFromBpm(-100), stepMsFromBpm(kMinBpm));
}

TEST(Tempo, StepMsMonotonicallyDecreases) {
    unsigned long prev = stepMsFromBpm(kMinBpm);
    for (int bpm = kMinBpm + 1; bpm <= kMaxBpm; ++bpm) {
        unsigned long cur = stepMsFromBpm(bpm);
        EXPECT_LE(cur, prev) << "non-monotonic at bpm=" << bpm;
        prev = cur;
    }
}

}  // namespace
