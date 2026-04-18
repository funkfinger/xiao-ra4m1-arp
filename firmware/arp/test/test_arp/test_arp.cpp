#include <gtest/gtest.h>

#include <set>
#include <vector>

#include "arp.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

namespace {

using arp::Arp;
using arp::Order;
using arp::orderFromPot;
using arp::kOrderHysteresis;

// Chord tones rooted at 48 (C3): A=48, B=52, C=55, D=60.
constexpr uint8_t A = 48;
constexpr uint8_t B = 52;
constexpr uint8_t C = 55;
constexpr uint8_t D = 60;

// ──────────────────────────────────────────────────────────────────────────
// Baseline — existing v0.1 behaviour.
// ──────────────────────────────────────────────────────────────────────────

TEST(Arp, DefaultOrderIsUp) {
    Arp a(48);
    EXPECT_EQ(a.order(), Order::Up);
}

TEST(Arp, InitialNoteIsRoot) {
    Arp a(48);
    EXPECT_EQ(a.current(), A);
}

TEST(Arp, UpPattern) {
    Arp a(48);
    EXPECT_EQ(a.current(), A);
    EXPECT_EQ(a.advance(), B);
    EXPECT_EQ(a.advance(), C);
    EXPECT_EQ(a.advance(), D);
    EXPECT_EQ(a.advance(), A);  // wrap
}

TEST(Arp, ResetReturnsToStepZero) {
    Arp a(48);
    a.advance();
    a.advance();
    a.reset();
    EXPECT_EQ(a.step(), 0);
    EXPECT_EQ(a.current(), A);
}

TEST(Arp, DifferentRoot) {
    Arp a(36);
    EXPECT_EQ(a.current(), 36);
    EXPECT_EQ(a.advance(), 40);
    EXPECT_EQ(a.advance(), 43);
    EXPECT_EQ(a.advance(), 48);
    EXPECT_EQ(a.advance(), 36);
}

// ──────────────────────────────────────────────────────────────────────────
// Down order.
// ──────────────────────────────────────────────────────────────────────────

TEST(Arp, DownPattern) {
    Arp a(48, Order::Down);
    EXPECT_EQ(a.current(), D);
    EXPECT_EQ(a.advance(), C);
    EXPECT_EQ(a.advance(), B);
    EXPECT_EQ(a.advance(), A);
    EXPECT_EQ(a.advance(), D);  // wrap
}

// ──────────────────────────────────────────────────────────────────────────
// UpDown / DownUp — 6-step pingpong.
// ──────────────────────────────────────────────────────────────────────────

TEST(Arp, UpDownPattern) {
    Arp a(48, Order::UpDown);
    std::vector<uint8_t> expected = {A, B, C, D, C, B, A, B, C, D, C, B};
    EXPECT_EQ(a.current(), expected[0]);
    for (size_t i = 1; i < expected.size(); ++i) {
        EXPECT_EQ(a.advance(), expected[i]) << "step " << i;
    }
}

TEST(Arp, UpDownLengthIsSix) {
    Arp a(48, Order::UpDown);
    EXPECT_EQ(a.patternLength(), 6);
}

TEST(Arp, DownUpPattern) {
    Arp a(48, Order::DownUp);
    std::vector<uint8_t> expected = {D, C, B, A, B, C, D, C, B, A, B, C};
    EXPECT_EQ(a.current(), expected[0]);
    for (size_t i = 1; i < expected.size(); ++i) {
        EXPECT_EQ(a.advance(), expected[i]) << "step " << i;
    }
}

// ──────────────────────────────────────────────────────────────────────────
// Skip — ACBD inside-out.
// ──────────────────────────────────────────────────────────────────────────

TEST(Arp, SkipPattern) {
    Arp a(48, Order::Skip);
    EXPECT_EQ(a.current(), A);
    EXPECT_EQ(a.advance(), C);
    EXPECT_EQ(a.advance(), B);
    EXPECT_EQ(a.advance(), D);
    EXPECT_EQ(a.advance(), A);  // wrap
}

// ──────────────────────────────────────────────────────────────────────────
// Random — deterministic given seed, always in-chord.
// ──────────────────────────────────────────────────────────────────────────

TEST(Arp, RandomIsReproducibleWithSeed) {
    Arp a1(48, Order::Random); a1.setSeed(42);
    Arp a2(48, Order::Random); a2.setSeed(42);
    for (int i = 0; i < 20; ++i) {
        EXPECT_EQ(a1.advance(), a2.advance()) << "step " << i;
    }
}

TEST(Arp, RandomOutputIsAlwaysInChord) {
    Arp a(48, Order::Random); a.setSeed(1);
    std::set<uint8_t> chord{A, B, C, D};
    for (int i = 0; i < 500; ++i) {
        uint8_t note = a.advance();
        EXPECT_TRUE(chord.count(note)) << "step " << i << " produced " << +note;
    }
}

TEST(Arp, RandomVisitsAllChordTones) {
    // With 500 draws over 4 possible values, every tone should appear.
    Arp a(48, Order::Random); a.setSeed(1);
    std::set<uint8_t> seen;
    for (int i = 0; i < 500; ++i) {
        seen.insert(a.advance());
    }
    EXPECT_EQ(seen.size(), 4u);
}

TEST(Arp, DifferentSeedsDifferentSequences) {
    Arp a1(48, Order::Random); a1.setSeed(1);
    Arp a2(48, Order::Random); a2.setSeed(2);
    std::vector<uint8_t> seq1, seq2;
    for (int i = 0; i < 20; ++i) {
        seq1.push_back(a1.advance());
        seq2.push_back(a2.advance());
    }
    EXPECT_NE(seq1, seq2);  // overwhelmingly likely
}

// ──────────────────────────────────────────────────────────────────────────
// setOrder behaviour.
// ──────────────────────────────────────────────────────────────────────────

TEST(Arp, SetOrderRestartsPattern) {
    Arp a(48, Order::Up);
    a.advance();  // → B
    a.advance();  // → C
    EXPECT_EQ(a.step(), 2);

    a.setOrder(Order::Down);
    EXPECT_EQ(a.step(), 0);
    EXPECT_EQ(a.current(), D);  // Down starts at D
}

TEST(Arp, SetOrderToSameIsNoop) {
    Arp a(48, Order::Up);
    a.advance();  // step=1
    a.advance();  // step=2
    a.setOrder(Order::Up);
    EXPECT_EQ(a.step(), 2);  // not reset
}

TEST(Arp, AllOrdersProduceChordTones) {
    std::set<uint8_t> chord{A, B, C, D};
    for (int o = 0; o < static_cast<int>(Order::Count); ++o) {
        Arp a(48, static_cast<Order>(o));
        a.setSeed(123);
        for (int i = 0; i < 20; ++i) {
            uint8_t note = a.advance();
            EXPECT_TRUE(chord.count(note))
                << "order=" << o << " step=" << i << " note=" << +note;
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────
// orderFromPot — zone mapping and hysteresis.
// ──────────────────────────────────────────────────────────────────────────

TEST(OrderFromPot, PotZeroIsUp) {
    EXPECT_EQ(orderFromPot(0.0f, Order::Up), Order::Up);
    EXPECT_EQ(orderFromPot(0.0f, Order::Random), Order::Up);
}

TEST(OrderFromPot, PotOneIsRandom) {
    EXPECT_EQ(orderFromPot(1.0f, Order::Random), Order::Random);
    EXPECT_EQ(orderFromPot(1.0f, Order::Up), Order::Random);
}

TEST(OrderFromPot, ZoneCenters) {
    constexpr float w = 1.0f / 6.0f;
    EXPECT_EQ(orderFromPot(w * 0.5f, Order::Random), Order::Up);
    EXPECT_EQ(orderFromPot(w * 1.5f, Order::Random), Order::Down);
    EXPECT_EQ(orderFromPot(w * 2.5f, Order::Random), Order::UpDown);
    EXPECT_EQ(orderFromPot(w * 3.5f, Order::Up),     Order::DownUp);
    EXPECT_EQ(orderFromPot(w * 4.5f, Order::Up),     Order::Skip);
    EXPECT_EQ(orderFromPot(w * 5.5f, Order::Up),     Order::Random);
}

TEST(OrderFromPot, SweepVisitsAllSixOrders) {
    std::vector<Order> visited;
    Order current = Order::Up;
    visited.push_back(current);
    for (float p = 0.0f; p <= 1.0f; p += 0.001f) {
        Order next = orderFromPot(p, current);
        if (next != current) {
            visited.push_back(next);
            current = next;
        }
    }
    ASSERT_EQ(visited.size(), 6u);
    EXPECT_EQ(visited[0], Order::Up);
    EXPECT_EQ(visited[1], Order::Down);
    EXPECT_EQ(visited[2], Order::UpDown);
    EXPECT_EQ(visited[3], Order::DownUp);
    EXPECT_EQ(visited[4], Order::Skip);
    EXPECT_EQ(visited[5], Order::Random);
}

TEST(OrderFromPot, JitterAtBoundaryDoesNotFlip) {
    constexpr float boundary = 1.0f / 6.0f;
    Order current = Order::Up;
    for (int i = 0; i < 100; ++i) {
        float noise = ((i % 2) == 0) ? 0.005f : -0.005f;
        Order next = orderFromPot(boundary + noise, current);
        EXPECT_EQ(next, Order::Up);
        current = next;
    }
}

TEST(OrderFromPot, ClampsOutOfRange) {
    EXPECT_EQ(orderFromPot(-0.5f, Order::Random), Order::Up);
    EXPECT_EQ(orderFromPot(1.5f,  Order::Up),     Order::Random);
}

}  // namespace
