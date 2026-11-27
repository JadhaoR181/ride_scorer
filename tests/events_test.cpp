#include <gtest/gtest.h>

#include <vector>

#include "ride/events.hpp"
#include "ride/smoother.hpp"
#include "ride/summary.hpp"

using ride::DetectorConfig;
using ride::Event;
using ride::EventType;
using ride::Motion;

namespace {

// Builds motion points at 1 Hz, 10 m/s, from lists of accelerations.
std::vector<Motion> make(const std::vector<double>& long_acc,
                         const std::vector<double>& lat_acc = {}) {
    std::vector<Motion> m;
    for (std::size_t i = 0; i < long_acc.size(); ++i) {
        const double lat = i < lat_acc.size() ? lat_acc[i] : 0.0;
        m.push_back({static_cast<double>(i), 10.0, long_acc[i], lat});
    }
    return m;
}

}  // namespace

TEST(Smooth, ConstantSignalStaysConstant) {
    const auto out = ride::smooth(make({1, 1, 1, 1}), 3);
    for (const auto& p : out) EXPECT_NEAR(p.long_acc, 1.0, 1e-9);
}

TEST(Smooth, SingleSpikeIsReduced) {
    const auto out = ride::smooth(make({0, 0, 6, 0, 0}), 3);
    EXPECT_NEAR(out[2].long_acc, 2.0, 1e-9);
}

TEST(Events, SmoothDrivingGivesNoEvents) {
    EXPECT_TRUE(ride::detect_events(make({0.5, -0.5, 1.0, -1.0, 0.2, 0.0})).empty());
}

TEST(Events, SustainedHardBrakeGivesOneEvent) {
    const auto e = ride::detect_events(make({0, -4, -5, -4, 0, 0}));
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, EventType::HarshBraking);
    EXPECT_NEAR(e[0].start_s, 1.0, 1e-9);
    EXPECT_NEAR(e[0].end_s, 3.0, 1e-9);
    EXPECT_NEAR(e[0].peak, -5.0, 1e-9);
}

TEST(Events, SingleSampleSpikeIsIgnored) {
    EXPECT_TRUE(ride::detect_events(make({0, -6, 0, 0})).empty());
}

TEST(Events, CooldownSuppressesImmediateRetrigger) {
    // Second burst starts 1 s after the first ends, inside the 3 s cooldown.
    const auto e = ride::detect_events(make({0, -4, -4, 0, -4, -4, 0, 0, 0}));
    EXPECT_EQ(e.size(), 1u);
}

TEST(Events, SeparatedBurstsGiveTwoEvents) {
    const auto e = ride::detect_events(make({0, -4, -4, 0, 0, 0, 0, 0, -4, -4, 0}));
    EXPECT_EQ(e.size(), 2u);
}

TEST(Events, HardCorneringUsesAbsoluteLateralValue) {
    const auto e = ride::detect_events(make({0, 0, 0, 0}, {0, -3.5, -3.5, 0}));
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, EventType::HardCornering);
    EXPECT_NEAR(e[0].peak, 3.5, 1e-9);
}

TEST(Summary, DistanceComesFromSpeedAndTime) {
    const auto s = ride::summarize(make({0, 0, 0, 0}), {});
    EXPECT_NEAR(s.distance_km, 0.03, 1e-9);  // 3 intervals x 10 m
    EXPECT_NEAR(s.duration_s, 3.0, 1e-9);
}

TEST(Summary, NoEventsGivesPerfectScore) {
    EXPECT_EQ(ride::summarize(make({0, 0, 0, 0}), {}).score, 100);
}

TEST(Summary, HarshBrakeLowersScoreAndIsCounted) {
    const std::vector<Event> events = {{EventType::HarshBraking, 1.0, 2.0, -4.0}};
    const auto s = ride::summarize(make({0, 0, 0, 0}), events);
    EXPECT_EQ(s.harsh_brakes, 1);
    EXPECT_LT(s.score, 100);
}