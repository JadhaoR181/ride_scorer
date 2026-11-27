#pragma once
#include <string>
#include <vector>
#include "ride/sample.hpp"

namespace ride {

// Reads a CSV with header: time_s,speed_mps,heading_deg
// Throws std::runtime_error if the file can't be opened.
// Skips lines that can't be parsed.
std::vector<Sample> read_samples(const std::string& path);

}  // namespace ride