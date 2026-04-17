#include <gtest/gtest.h>

#include "arp.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace {

using arp::Arp;

TEST(Arp, InitialNoteIsRoot) {
    Arp a(48);
    EXPECT_EQ(a.current(), 48);  // C3
}

TEST(Arp, AdvanceProducesUpPattern) {
    Arp a(48);
    EXPECT_EQ(a.current(), 48);   // root (C3)
    EXPECT_EQ(a.advance(), 52);   // major 3rd (E3)
    EXPECT_EQ(a.advance(), 55);   // 5th (G3)
    EXPECT_EQ(a.advance(), 60);   // octave (C4)
}

TEST(Arp, WrapsAfterLastNote) {
    Arp a(48);
    a.advance();  // → E3
    a.advance();  // → G3
    a.advance();  // → C4
    EXPECT_EQ(a.advance(), 48);  // wraps back to root
}

TEST(Arp, TwoFullCycles) {
    Arp a(60);  // root = C4
    uint8_t expected[] = {60, 64, 67, 72, 60, 64, 67, 72};
    EXPECT_EQ(a.current(), expected[0]);
    for (int i = 1; i < 8; ++i) {
        EXPECT_EQ(a.advance(), expected[i]) << "step " << i;
    }
}

TEST(Arp, ResetReturnsToStepZero) {
    Arp a(48);
    a.advance();
    a.advance();
    EXPECT_EQ(a.step(), 2);
    a.reset();
    EXPECT_EQ(a.step(), 0);
    EXPECT_EQ(a.current(), 48);
}

TEST(Arp, ResetFromAnyPosition) {
    Arp a(48);
    a.advance();
    a.advance();
    a.advance();
    EXPECT_EQ(a.current(), 60);  // C4
    a.reset();
    EXPECT_EQ(a.current(), 48);  // back to C3
    EXPECT_EQ(a.advance(), 52);  // next is E3, not C4
}

TEST(Arp, DifferentRoot) {
    Arp a(36);  // C2
    EXPECT_EQ(a.current(), 36);
    EXPECT_EQ(a.advance(), 40);   // E2
    EXPECT_EQ(a.advance(), 43);   // G2
    EXPECT_EQ(a.advance(), 48);   // C3
    EXPECT_EQ(a.advance(), 36);   // wraps
}

TEST(Arp, StepCounterTracksPosition) {
    Arp a(48);
    EXPECT_EQ(a.step(), 0);
    a.advance();
    EXPECT_EQ(a.step(), 1);
    a.advance();
    EXPECT_EQ(a.step(), 2);
    a.advance();
    EXPECT_EQ(a.step(), 3);
    a.advance();
    EXPECT_EQ(a.step(), 0);  // wrapped
}

TEST(Arp, NumNotesIsFour) {
    Arp a(48);
    EXPECT_EQ(a.numNotes(), 4);
}

}  // namespace
