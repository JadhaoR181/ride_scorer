#pragma once
#include <vector>
#include "ride/sample.hpp"

namespace ride {

enum class EventType { HarshBraking, HarshAcceleration, HardCornering };

struct Event {
    EventType type;
    double start_s;  // time of first sample over the threshold
    double end_s;    // time of last sample over the threshold
    double peak;     // strongest value (m/s^2); braking is negative
};

// Starting values only. Tune them on your own rides.
struct DetectorConfig {
    double brake_threshold = -3.0;   // longitudinal acc at or below this = braking
    double accel_threshold = 2.5;    // longitudinal acc at or above this = harsh accel
    double corner_threshold = 3.0;   // |lateral acc| at or above this = hard corner
    int min_samples = 2;             // must persist this many samples (filters GPS noise)
    double cooldown_s = 3.0;         // ignore re-triggers right after an event
};

const char* to_string(EventType type);

// Runs one Normal -> InEvent -> Cooldown state machine per channel.
// Returns all events sorted by start time.
std::vector<Event> detect_events(const std::vector<Motion>& motion,
                                 const DetectorConfig& cfg = DetectorConfig{});

}  // namespace ride