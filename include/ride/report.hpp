#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include "ride/events.hpp"
#include "ride/sample.hpp"
#include "ride/summary.hpp"

namespace ride {

// Trip result as JSON (summary, detector settings used, event list).
std::string to_json(const TripSummary& summary, const std::vector<Event>& events,
                    const DetectorConfig& cfg, std::size_t window);

// Smoothed motion series as CSV, used by the plotting script.
std::string motion_to_csv(const std::vector<Motion>& motion);

}  // namespace ride