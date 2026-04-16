#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <vector>

#include "scales.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace {

using arp::Scale;
using arp::quantize;

constexpr uint8_t kMidiMax = 127;

const std::vector<uint8_t> kMajor            = {0, 2, 4, 5, 7, 9, 11};
const std::vector<uint8_t> kNaturalMinor     = {0, 2, 3, 5, 7, 8, 10};
const std::vector<uint8_t> kPentatonicMajor  = {0, 2, 4, 7, 9};
const std::vector<uint8_t> kPentatonicMinor  = {0, 3, 5, 7, 10};
const std::vector<uint8_t> kDorian           = {0, 2, 3, 5, 7, 9, 10};

std::set<uint8_t> pitch_class_set(const std::vector<uint8_t>& intervals) {
    return {intervals.begin(), intervals.end()};
}

}  // namespace

// ──────────────────────────────────────────────────────────────────────────
// Criterion: In-scale notes pass through unchanged.
// ──────────────────────────────────────────────────────────────────────────

TEST(Quantize, InScalePassthrough_Major) {
    auto pcs = pitch_class_set(kMajor);
    for (uint8_t n = 0; n <= kMidiMax; ++n) {
        if (pcs.count(n % 12u)) EXPECT_EQ(quantize(n, Scale::Major), n) << "n=" << +n;
    }
}

TEST(Quantize, InScalePassthrough_NaturalMinor) {
    auto pcs = pitch_class_set(kNaturalMinor);
    for (uint8_t n = 0; n <= kMidiMax; ++n) {
        if (pcs.count(n % 12u)) EXPECT_EQ(quantize(n, Scale::NaturalMinor), n) << "n=" << +n;
    }
}

TEST(Quantize, InScalePassthrough_PentatonicMajor) {
    auto pcs = pitch_class_set(kPentatonicMajor);
    for (uint8_t n = 0; n <= kMidiMax; ++n) {
        if (pcs.count(n % 12u)) EXPECT_EQ(quantize(n, Scale::PentatonicMajor), n) << "n=" << +n;
    }
}

TEST(Quantize, InScalePassthrough_PentatonicMinor) {
    auto pcs = pitch_class_set(kPentatonicMinor);
    for (uint8_t n = 0; n <= kMidiMax; ++n) {
        if (pcs.count(n % 12u)) EXPECT_EQ(quantize(n, Scale::PentatonicMinor), n) << "n=" << +n;
    }
}

TEST(Quantize, InScalePassthrough_Dorian) {
    auto pcs = pitch_class_set(kDorian);
    for (uint8_t n = 0; n <= kMidiMax; ++n) {
        if (pcs.count(n % 12u)) EXPECT_EQ(quantize(n, Scale::Dorian), n) << "n=" << +n;
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Criterion: Chromatic is identity for all MIDI notes.
// ──────────────────────────────────────────────────────────────────────────

TEST(Quantize, ChromaticIsIdentity) {
    for (uint8_t n = 0; n <= kMidiMax; ++n) {
        EXPECT_EQ(quantize(n, Scale::Chromatic), n) << "n=" << +n;
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Criterion: Octave invariance — quantize(n,s) + 12 == quantize(n+12,s).
// Holds only where both inputs and outputs fit in [0, 127].
// ──────────────────────────────────────────────────────────────────────────

void expect_octave_invariant(Scale s) {
    for (uint8_t n = 0; n + 12 <= kMidiMax; ++n) {
        const auto lo = quantize(n, s);
        const auto hi = quantize(static_cast<uint8_t>(n + 12), s);
        if (lo + 12 <= kMidiMax) {
            EXPECT_EQ(lo + 12, hi) << "n=" << +n << " scale=" << static_cast<int>(s);
        }
    }
}

TEST(Quantize, OctaveInvariance_Major)            { expect_octave_invariant(Scale::Major); }
TEST(Quantize, OctaveInvariance_NaturalMinor)     { expect_octave_invariant(Scale::NaturalMinor); }
TEST(Quantize, OctaveInvariance_PentatonicMajor)  { expect_octave_invariant(Scale::PentatonicMajor); }
TEST(Quantize, OctaveInvariance_PentatonicMinor)  { expect_octave_invariant(Scale::PentatonicMinor); }
TEST(Quantize, OctaveInvariance_Dorian)           { expect_octave_invariant(Scale::Dorian); }

// ──────────────────────────────────────────────────────────────────────────
// Criterion: Ties break downward (Major whole-tone gaps).
// In C major, C#/D#/F#/G#/A# are all equidistant from their neighbours.
// ──────────────────────────────────────────────────────────────────────────

TEST(Quantize, TiesBreakDownward_Major) {
    EXPECT_EQ(quantize(61, Scale::Major), 60);  // C# → C
    EXPECT_EQ(quantize(63, Scale::Major), 62);  // D# → D
    EXPECT_EQ(quantize(66, Scale::Major), 65);  // F# → F
    EXPECT_EQ(quantize(68, Scale::Major), 67);  // G# → G
    EXPECT_EQ(quantize(70, Scale::Major), 69);  // A# → A
}

// ──────────────────────────────────────────────────────────────────────────
// Criterion: Out-of-scale notes map to nearest in-scale tone
// (not floor) — pentatonic demonstrates the distinction.
// ──────────────────────────────────────────────────────────────────────────

TEST(Quantize, NearestNotFloor_PentatonicMinor) {
    // C pentatonic minor: 0, 3, 5, 7, 10.
    // D (62) is dist=2 from C (60), dist=1 from E♭ (63). Nearest is 63.
    EXPECT_EQ(quantize(62, Scale::PentatonicMinor), 63);
    // F# (66) is dist=1 from F (65)... but F is NOT in pentatonic minor.
    // Nearest in-scale: 65? No — 65 is F, pitch class 5, which IS in the scale.
    EXPECT_EQ(quantize(66, Scale::PentatonicMinor), 65);
    // A (69): pitch class 9. Pentatonic minor has 7 and 10. Dist to 67 (7) = 2, dist to 70 (10) = 1. → 70.
    EXPECT_EQ(quantize(69, Scale::PentatonicMinor), 70);
}

TEST(Quantize, NearestNotFloor_PentatonicMajor) {
    // C pentatonic major: 0, 2, 4, 7, 9.
    // F (65, pc=5): dist to E (64, pc=4) = 1, dist to G (67, pc=7) = 2. Nearest = E (64).
    EXPECT_EQ(quantize(65, Scale::PentatonicMajor), 64);
    // B (71, pc=11): dist to A (69, pc=9) = 2, dist to next-octave-C (72, pc=0) = 1. Nearest = 72.
    EXPECT_EQ(quantize(71, Scale::PentatonicMajor), 72);
}

// ──────────────────────────────────────────────────────────────────────────
// Boundary behaviour.
// ──────────────────────────────────────────────────────────────────────────

TEST(Quantize, Boundary_Zero_AllScales) {
    // MIDI 0 (C-1) is the root of every scale — must pass through.
    for (int s = 0; s < static_cast<int>(Scale::Count); ++s) {
        EXPECT_EQ(quantize(0, static_cast<Scale>(s)), 0) << "scale=" << s;
    }
}

TEST(Quantize, Boundary_127_Major) {
    // MIDI 127 (G9): pc=7 is in major; passthrough.
    EXPECT_EQ(quantize(127, Scale::Major), 127);
}

TEST(Quantize, Boundary_127_NaturalMinor) {
    // MIDI 127: pc=7 is in natural minor; passthrough.
    EXPECT_EQ(quantize(127, Scale::NaturalMinor), 127);
}

TEST(Quantize, Boundary_126_Major) {
    // MIDI 126: pc=6 (F#), NOT in major. Nearest: F (125) and G (127), tie → F (125).
    EXPECT_EQ(quantize(126, Scale::Major), 125);
}

// ──────────────────────────────────────────────────────────────────────────
// Invariant: output is always in-scale (never produces out-of-scale values).
// ──────────────────────────────────────────────────────────────────────────

void expect_output_always_in_scale(Scale s, const std::set<uint8_t>& pcs) {
    for (uint8_t n = 0; n <= kMidiMax; ++n) {
        const auto q = quantize(n, s);
        EXPECT_TRUE(pcs.count(q % 12u)) << "n=" << +n << " quantised to out-of-scale " << +q;
        EXPECT_LE(q, kMidiMax) << "quantised note exceeds MIDI range";
    }
}

TEST(Quantize, OutputAlwaysInScale_Major)            { expect_output_always_in_scale(Scale::Major, pitch_class_set(kMajor)); }
TEST(Quantize, OutputAlwaysInScale_NaturalMinor)     { expect_output_always_in_scale(Scale::NaturalMinor, pitch_class_set(kNaturalMinor)); }
TEST(Quantize, OutputAlwaysInScale_PentatonicMajor)  { expect_output_always_in_scale(Scale::PentatonicMajor, pitch_class_set(kPentatonicMajor)); }
TEST(Quantize, OutputAlwaysInScale_PentatonicMinor)  { expect_output_always_in_scale(Scale::PentatonicMinor, pitch_class_set(kPentatonicMinor)); }
TEST(Quantize, OutputAlwaysInScale_Dorian)           { expect_output_always_in_scale(Scale::Dorian, pitch_class_set(kDorian)); }
