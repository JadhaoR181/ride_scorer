#pragma once

namespace ride {

// One raw GPS reading.
struct Sample {
    double t_s;          // time in seconds
    double speed_mps;    // speed in metres per second
    double heading_deg;  // direction of travel, 0-360 degrees
};

// What we compute from two consecutive samples.
struct Motion {
    double t_s;
    double speed_mps;
    double long_acc;  // forward/back acceleration, m/s^2 (negative = braking)
    double lat_acc;   // sideways acceleration, m/s^2 (positive = right turn)
};

}  // namespace ride