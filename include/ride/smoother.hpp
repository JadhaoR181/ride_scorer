#pragma once
#include <cstddef>
#include <vector>
#include "ride/sample.hpp"

namespace ride {

// Trailing moving average of long_acc and lat_acc over `window` points.
// A window below 1 is treated as 1. The first points average what is available.
std::vector<Motion> smooth(const std::vector<Motion>& in, std::size_t window);

}  // namespace ride