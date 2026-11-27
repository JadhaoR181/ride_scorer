#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "ride/report.hpp"

using ride::Event;
using ride::EventType;
using ride::Motion;

namespace {

std::vector<Motion> steady(std::size_t n) {
    std::vector<Motion> m;
    for (std::size_t i = 0; i < n; ++i) m.push_back({static_cast<double>(i), 10.0, 0.0, 0.0});
    return m;
}

}  // namespace

TEST(Report, JsonListsEventsAndCounts) {
    const std::vector<Event> events = {{EventType::HarshBraking, 1.0, 2.0, -4.0}};
    const auto motion = steady(4);
    const std::string json = ride::to_json(ride::summarize(motion, events), events,
                                           ride::DetectorConfig{}, 3);
    EXPECT_NE(json.find("\"harsh_braking\""), std::string::npos);
    EXPECT_NE(json.find("\"harsh_brakes\": 1"), std::string::npos);
}

TEST(Report, JsonWithNoEventsHasEmptyList) {
    const auto motion = steady(4);
    const std::string json = ride::to_json(ride::summarize(motion, {}), {},
                                           ride::DetectorConfig{}, 3);
    EXPECT_NE(json.find("\"events\": []"), std::string::npos);
    EXPECT_NE(json.find("\"score\": 100"), std::string::npos);
}

TEST(Report, MotionCsvHasHeaderAndOneLinePerPoint) {
    const std::string csv = ride::motion_to_csv(steady(5));
    EXPECT_EQ(std::count(csv.begin(), csv.end(), '\n'), 6);  // header + 5 rows
}

TEST(Summary, IdleShareAndMaxSpeed) {
    const std::vector<Motion> m = {{0, 0, 0, 0}, {1, 0, 0, 0}, {2, 10, 0, 0}, {3, 10, 0, 0}};
    const auto s = ride::summarize(m, {});
    EXPECT_NEAR(s.idle_share, 1.0 / 3.0, 1e-9);
    EXPECT_NEAR(s.max_speed_kmh, 36.0, 1e-9);
}