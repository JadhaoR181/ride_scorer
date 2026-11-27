#include "ride/summary.hpp"

#include <algorithm>
#include <cmath>

namespace ride {

namespace {
// Points lost per event per km. Starting values, tune them on real rides.
constexpr double kBrakePenalty = 60.0;
constexpr double kAccelPenalty = 30.0;
constexpr double kCornerPenalty = 40.0;
// Trips shorter than this are scored as if they were this long.
constexpr double kMinKm = 1.0;
// Below this speed the bike counts as stopped/idling.
constexpr double kIdleSpeedMps = 1.0;
}  // namespace

TripSummary summarize(const std::vector<Motion>& motion, const std::vector<Event>& events) {
    TripSummary s{0.0, 0.0, 0.0, 0, 0, 0, 100, 0.0, 0.0};
    if (motion.empty()) return s;

    double distance_m = 0.0;
    double idle_s = 0.0;
    double max_speed_mps = motion.front().speed_mps;
    for (std::size_t i = 1; i < motion.size(); ++i) {
        const double dt = motion[i].t_s - motion[i - 1].t_s;
        distance_m += motion[i].speed_mps * dt;
        if (motion[i].speed_mps < kIdleSpeedMps) idle_s += dt;
        max_speed_mps = std::max(max_speed_mps, motion[i].speed_mps);
    }
    s.distance_km = distance_m / 1000.0;
    s.duration_s = motion.back().t_s - motion.front().t_s;
    s.max_speed_kmh = max_speed_mps * 3.6;
    if (s.duration_s > 0.0) {
        s.avg_speed_kmh = (distance_m / s.duration_s) * 3.6;
        s.idle_share = idle_s / s.duration_s;
    }

    for (const Event& e : events) {
        switch (e.type) {
            case EventType::HarshBraking: ++s.harsh_brakes; break;
            case EventType::HarshAcceleration: ++s.harsh_accels; break;
            case EventType::HardCornering: ++s.hard_corners; break;
        }
    }

    const double km = std::max(s.distance_km, kMinKm);
    const double penalty = (kBrakePenalty * s.harsh_brakes + kAccelPenalty * s.harsh_accels +
                            kCornerPenalty * s.hard_corners) / km;
    s.score = static_cast<int>(std::lround(std::clamp(100.0 - penalty, 0.0, 100.0)));
    return s;
}

}  // namespace ride