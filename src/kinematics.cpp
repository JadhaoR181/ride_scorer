#include "ride/kinematics.hpp"

namespace ride {

namespace {
constexpr double kPi = 3.14159265358979323846;
}

double wrap_deg(double d) {
    while (d > 180.0) d -= 360.0;
    while (d <= -180.0) d += 360.0;
    return d;
}

std::vector<Motion> compute_motion(const std::vector<Sample>& s) {
    std::vector<Motion> out;
    if (s.size() < 2) return out;
    out.reserve(s.size() - 1);

    for (std::size_t i = 1; i < s.size(); ++i) {
        const double dt = s[i].t_s - s[i - 1].t_s;
        if (dt <= 0.0) continue;  // bad or duplicate timestamp

        const double long_acc = (s[i].speed_mps - s[i - 1].speed_mps) / dt;
        const double yaw_rate =
            wrap_deg(s[i].heading_deg - s[i - 1].heading_deg) * kPi / 180.0 / dt;
        const double lat_acc = s[i].speed_mps * yaw_rate;

        out.push_back({s[i].t_s, s[i].speed_mps, long_acc, lat_acc});
    }
    return out;
}

}  // namespace ride