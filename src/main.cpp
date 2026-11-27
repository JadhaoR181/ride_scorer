#include <algorithm>
#include <iostream>

#include "ride/csv_reader.hpp"
#include "ride/kinematics.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ride_cli <ride.csv>\n";
        return 1;
    }
    try {
        const auto samples = ride::read_samples(argv[1]);
        const auto motion = ride::compute_motion(samples);

        std::cout << "Samples: " << samples.size()
                  << ", motion points: " << motion.size() << "\n";
        if (motion.empty()) return 0;

        const auto cmp_long = [](const ride::Motion& a, const ride::Motion& b) {
            return a.long_acc < b.long_acc;
        };
        const auto hardest_brake =
            *std::min_element(motion.begin(), motion.end(), cmp_long);
        const auto hardest_accel =
            *std::max_element(motion.begin(), motion.end(), cmp_long);

        std::cout << "Hardest braking: " << hardest_brake.long_acc
                  << " m/s^2 at t=" << hardest_brake.t_s << "s\n";
        std::cout << "Hardest acceleration: " << hardest_accel.long_acc
                  << " m/s^2 at t=" << hardest_accel.t_s << "s\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}