#include <gtest/gtest.h>

#include <string>

#include "ride/csv_reader.hpp"
#include "ride/events.hpp"
#include "ride/kinematics.hpp"
#include "ride/smoother.hpp"
#include "ride/summary.hpp"

TEST(Pipeline, SyntheticRideProducesTheThreePlantedEvents) {
    const std::string path = std::string(RIDE_DATA_DIR) + "/synthetic_ride_01.csv";
    const auto samples = ride::read_samples(path);
    ASSERT_GT(samples.size(), 200u);

    const auto motion = ride::smooth(ride::compute_motion(samples), 3);
    const auto events = ride::detect_events(motion);

    ASSERT_EQ(events.size(), 3u);
    EXPECT_EQ(events[0].type, ride::EventType::HarshAcceleration);
    EXPECT_EQ(events[1].type, ride::EventType::HardCornering);
    EXPECT_EQ(events[2].type, ride::EventType::HarshBraking);

    const auto s = ride::summarize(motion, events);
    EXPECT_EQ(s.harsh_accels, 1);
    EXPECT_EQ(s.hard_corners, 1);
    EXPECT_EQ(s.harsh_brakes, 1);
    EXPECT_LT(s.score, 100);
}