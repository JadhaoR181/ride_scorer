#include "ride/events.hpp"

#include <algorithm>
#include <cmath>
#include <functional>

namespace ride {

namespace {

enum class State { Normal, InEvent, Cooldown };

// One thing to watch: which value, which direction, which threshold.
struct Channel {
    EventType type;
    std::function<double(const Motion&)> value;
    bool trigger_below;  // true: "over the threshold" means value <= threshold
    double threshold;
};

std::vector<Event> scan(const std::vector<Motion>& motion, const Channel& ch,
                        const DetectorConfig& cfg) {
    std::vector<Event> events;
    State state = State::Normal;
    double start = 0.0, last = 0.0, peak = 0.0, cooldown_end = 0.0;
    int count = 0;

    const auto begin_event = [&](const Motion& p, double v) {
        state = State::InEvent;
        start = last = p.t_s;
        peak = v;
        count = 1;
    };
    const auto close_event = [&]() {
        if (count >= cfg.min_samples) {
            events.push_back({ch.type, start, last, peak});
        }
    };

    for (const Motion& p : motion) {
        const double v = ch.value(p);
        const bool over = ch.trigger_below ? (v <= ch.threshold) : (v >= ch.threshold);

        switch (state) {
            case State::Normal:
                if (over) begin_event(p, v);
                break;
            case State::InEvent:
                if (over) {
                    last = p.t_s;
                    ++count;
                    peak = ch.trigger_below ? std::min(peak, v) : std::max(peak, v);
                } else {
                    close_event();
                    state = State::Cooldown;
                    cooldown_end = p.t_s + cfg.cooldown_s;
                }
                break;
            case State::Cooldown:
                if (p.t_s >= cooldown_end) {
                    state = State::Normal;
                    if (over) begin_event(p, v);
                }
                break;
        }
    }
    if (state == State::InEvent) close_event();  // data ended mid-event
    return events;
}

}  // namespace

const char* to_string(EventType type) {
    switch (type) {
        case EventType::HarshBraking: return "Harsh braking";
        case EventType::HarshAcceleration: return "Harsh acceleration";
        case EventType::HardCornering: return "Hard cornering";
    }
    return "Unknown";
}

std::vector<Event> detect_events(const std::vector<Motion>& motion,
                                 const DetectorConfig& cfg) {
    const std::vector<Channel> channels = {
        {EventType::HarshBraking, [](const Motion& m) { return m.long_acc; }, true,
         cfg.brake_threshold},
        {EventType::HarshAcceleration, [](const Motion& m) { return m.long_acc; }, false,
         cfg.accel_threshold},
        {EventType::HardCornering, [](const Motion& m) { return std::fabs(m.lat_acc); },
         false, cfg.corner_threshold},
    };

    std::vector<Event> all;
    for (const Channel& ch : channels) {
        const std::vector<Event> found = scan(motion, ch, cfg);
        all.insert(all.end(), found.begin(), found.end());
    }
    std::sort(all.begin(), all.end(),
              [](const Event& a, const Event& b) { return a.start_s < b.start_s; });
    return all;
}

}  // namespace ride