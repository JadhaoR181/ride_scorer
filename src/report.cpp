#include "ride/report.hpp"

#include <iomanip>
#include <sstream>

namespace ride {

namespace {

const char* id_of(EventType type) {
    switch (type) {
        case EventType::HarshBraking: return "harsh_braking";
        case EventType::HarshAcceleration: return "harsh_acceleration";
        case EventType::HardCornering: return "hard_cornering";
    }
    return "unknown";
}

}  // namespace

std::string to_json(const TripSummary& s, const std::vector<Event>& events,
                    const DetectorConfig& cfg, std::size_t window) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(3);
    o << "{\n";
    o << "  \"summary\": {\n";
    o << "    \"score\": " << s.score << ",\n";
    o << "    \"distance_km\": " << s.distance_km << ",\n";
    o << "    \"duration_s\": " << s.duration_s << ",\n";
    o << "    \"avg_speed_kmh\": " << s.avg_speed_kmh << ",\n";
    o << "    \"max_speed_kmh\": " << s.max_speed_kmh << ",\n";
    o << "    \"idle_share\": " << s.idle_share << ",\n";
    o << "    \"harsh_brakes\": " << s.harsh_brakes << ",\n";
    o << "    \"harsh_accels\": " << s.harsh_accels << ",\n";
    o << "    \"hard_corners\": " << s.hard_corners << "\n";
    o << "  },\n";
    o << "  \"config\": {\n";
    o << "    \"window\": " << window << ",\n";
    o << "    \"brake_threshold\": " << cfg.brake_threshold << ",\n";
    o << "    \"accel_threshold\": " << cfg.accel_threshold << ",\n";
    o << "    \"corner_threshold\": " << cfg.corner_threshold << "\n";
    o << "  },\n";
    o << "  \"events\": [";
    for (std::size_t i = 0; i < events.size(); ++i) {
        const Event& e = events[i];
        o << (i == 0 ? "\n" : ",\n");
        o << "    {\"type\": \"" << id_of(e.type) << "\", \"start_s\": " << e.start_s
          << ", \"end_s\": " << e.end_s << ", \"peak\": " << e.peak << "}";
    }
    o << (events.empty() ? "]\n" : "\n  ]\n");
    o << "}\n";
    return o.str();
}

std::string motion_to_csv(const std::vector<Motion>& motion) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(3);
    o << "time_s,speed_mps,long_acc,lat_acc\n";
    for (const Motion& p : motion) {
        o << p.t_s << ',' << p.speed_mps << ',' << p.long_acc << ',' << p.lat_acc << '\n';
    }
    return o.str();
}

}  // namespace ride