#include <iomanip>
#include <iostream>

#include "ride/csv_reader.hpp"
#include "ride/events.hpp"
#include "ride/kinematics.hpp"
#include "ride/smoother.hpp"
#include "ride/summary.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ride_cli <ride.csv>\n";
        return 1;
    }
    try {
        const auto samples = ride::read_samples(argv[1]);
        const auto raw = ride::compute_motion(samples);
        const auto motion = ride::smooth(raw, 3);
        const auto events = ride::detect_events(motion);
        const auto s = ride::summarize(motion, events);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Distance: " << s.distance_km << " km, duration: " << s.duration_s
                  << " s, avg speed: " << s.avg_speed_kmh << " km/h\n";
        std::cout << "Events: " << events.size() << "\n";
        for (const auto& e : events) {
            std::cout << "  " << ride::to_string(e.type) << " from t=" << e.start_s
                      << "s to t=" << e.end_s << "s, peak " << e.peak << " m/s^2\n";
        }
        std::cout << "Smoothness score: " << s.score << "/100\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}