#include <gtest/gtest.h>

#include "ride/kinematics.hpp"

using ride::Sample;

TEST(WrapDeg, HandlesWraparound) {
    EXPECT_NEAR(ride::wrap_deg(10.0 - 350.0), 20.0, 1e-9);
    EXPECT_NEAR(ride::wrap_deg(350.0 - 10.0), -20.0, 1e-9);
}

TEST(Motion, ConstantSpeedGivesZeroAcceleration) {
    std::vector<Sample> s = {{0, 10, 90}, {1, 10, 90}, {2, 10, 90}};
    auto m = ride::compute_motion(s);
    ASSERT_EQ(m.size(), 2u);
    EXPECT_NEAR(m[0].long_acc, 0.0, 1e-9);
    EXPECT_NEAR(m[0].lat_acc, 0.0, 1e-9);
}

TEST(Motion, SpeedingUpGivesPositiveLongitudinal) {
    std::vector<Sample> s = {{0, 0, 0}, {2, 10, 0}};
    auto m = ride::compute_motion(s);
    ASSERT_EQ(m.size(), 1u);
    EXPECT_NEAR(m[0].long_acc, 5.0, 1e-9);
}

TEST(Motion, BrakingGivesNegativeLongitudinal) {
    std::vector<Sample> s = {{0, 20, 0}, {1, 14, 0}};
    auto m = ride::compute_motion(s);
    ASSERT_EQ(m.size(), 1u);
    EXPECT_NEAR(m[0].long_acc, -6.0, 1e-9);
}

TEST(Motion, TurnAcrossNorthUsesWrappedHeading) {
    // 350 -> 10 degrees in 1 s at 10 m/s = 20 deg/s right turn
    std::vector<Sample> s = {{0, 10, 350}, {1, 10, 10}};
    auto m = ride::compute_motion(s);
    ASSERT_EQ(m.size(), 1u);
    EXPECT_NEAR(m[0].lat_acc, 10.0 * (20.0 * 3.14159265358979 / 180.0), 1e-6);
}

TEST(Motion, SkipsBadTimestamps) {
    std::vector<Sample> s = {{1, 5, 0}, {1, 6, 0}};
    EXPECT_TRUE(ride::compute_motion(s).empty());
}

TEST(Motion, LongGpsGapIsSkipped) {
    std::vector<Sample> s = {{0, 10, 0}, {10, 20, 0}};
    EXPECT_TRUE(ride::compute_motion(s).empty());
}

TEST(Motion, HeadingNoiseWhileStoppedGivesNoLateralAcceleration) {
    std::vector<Sample> s = {{0, 0, 0}, {1, 0, 90}};
    auto m = ride::compute_motion(s);
    ASSERT_EQ(m.size(), 1u);
    EXPECT_NEAR(m[0].lat_acc, 0.0, 1e-9);
}