#pragma once
#include <vector>
#include "ride/events.hpp"
#include "ride/sample.hpp"

namespace ride {

struct TripSummary {
    double distance_km;
    double duration_s;
    double avg_speed_kmh;
    int harsh_brakes;
    int harsh_accels;
    int hard_corners;
    int score;  // 0-100, higher is smoother
    double max_speed_kmh;
    double idle_share;  // fraction of trip time spent below ~1 m/s
};

TripSummary summarize(const std::vector<Motion>& motion, const std::vector<Event>& events);

}  // namespace ride