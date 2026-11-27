#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ride/csv_reader.hpp"
#include "ride/events.hpp"
#include "ride/kinematics.hpp"
#include "ride/report.hpp"
#include "ride/smoother.hpp"
#include "ride/summary.hpp"

namespace {

void print_usage() {
    std::cerr << "Usage: ride_cli <ride.csv> [options]\n"
                 "  --window N         smoothing window in samples (default 3)\n"
                 "  --brake X          braking threshold, m/s^2, negative (default -3.0)\n"
                 "  --accel X          acceleration threshold, m/s^2 (default 2.5)\n"
                 "  --corner X         cornering threshold, m/s^2 (default 3.0)\n"
                 "  --json PATH        write trip result as JSON\n"
                 "  --motion-csv PATH  write smoothed motion series as CSV\n";
}

void write_file(const std::string& path, const std::string& text) {
    std::ofstream f(path);
    if (!f.is_open()) throw std::runtime_error("Cannot write file: " + path);
    f << text;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    const std::string path = argv[1];
    ride::DetectorConfig cfg;
    std::size_t window = 3;
    std::string json_path;
    std::string motion_csv_path;

    try {
        for (int i = 2; i < argc; ++i) {
            const std::string arg = argv[i];
            if (i + 1 >= argc) throw std::runtime_error("Missing value for " + arg);
            const std::string val = argv[++i];
            if (arg == "--window") window = static_cast<std::size_t>(std::stoul(val));
            else if (arg == "--brake") cfg.brake_threshold = std::stod(val);
            else if (arg == "--accel") cfg.accel_threshold = std::stod(val);
            else if (arg == "--corner") cfg.corner_threshold = std::stod(val);
            else if (arg == "--json") json_path = val;
            else if (arg == "--motion-csv") motion_csv_path = val;
            else throw std::runtime_error("Unknown option: " + arg);
        }

        const auto samples = ride::read_samples(path);
        const auto raw = ride::compute_motion(samples);
        const auto motion = ride::smooth(raw, window);
        const auto events = ride::detect_events(motion, cfg);
        const auto s = ride::summarize(motion, events);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Distance: " << s.distance_km << " km, duration: " << s.duration_s
                  << " s, avg speed: " << s.avg_speed_kmh << " km/h, max speed: "
                  << s.max_speed_kmh << " km/h, idle: " << s.idle_share * 100.0 << "%\n";
        std::cout << "Events: " << events.size() << "\n";
        for (const auto& e : events) {
            std::cout << "  " << ride::to_string(e.type) << " from t=" << e.start_s
                      << "s to t=" << e.end_s << "s, peak " << e.peak << " m/s^2\n";
        }
        std::cout << "Smoothness score: " << s.score << "/100\n";

        if (!json_path.empty()) write_file(json_path, ride::to_json(s, events, cfg, window));
        if (!motion_csv_path.empty()) write_file(motion_csv_path, ride::motion_to_csv(motion));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}