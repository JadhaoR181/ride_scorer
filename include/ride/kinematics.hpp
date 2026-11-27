#pragma once
#include <vector>
#include "ride/sample.hpp"

namespace ride {

// Wraps an angle difference into (-180, 180] so 350 -> 10 degrees is +20, not -340.
double wrap_deg(double d);

// Longitudinal acceleration = change in speed / time.
// Lateral acceleration = speed * yaw rate (heading change per second).
// Both work with the phone in any orientation because they use GPS only.
std::vector<Motion> compute_motion(const std::vector<Sample>& samples);

}  // namespace ride