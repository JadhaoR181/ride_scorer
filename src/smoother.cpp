#include "ride/smoother.hpp"

namespace ride {

std::vector<Motion> smooth(const std::vector<Motion>& in, std::size_t window) {
    if (window < 1) window = 1;

    std::vector<Motion> out = in;
    double sum_long = 0.0;
    double sum_lat = 0.0;

    for (std::size_t i = 0; i < in.size(); ++i) {
        sum_long += in[i].long_acc;
        sum_lat += in[i].lat_acc;
        if (i >= window) {  // drop the value that just left the window
            sum_long -= in[i - window].long_acc;
            sum_lat -= in[i - window].lat_acc;
        }
        const std::size_t n = (i + 1 < window) ? i + 1 : window;
        out[i].long_acc = sum_long / static_cast<double>(n);
        out[i].lat_acc = sum_lat / static_cast<double>(n);
    }
    return out;
}

}  // namespace ride