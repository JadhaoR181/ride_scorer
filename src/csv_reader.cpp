#include "ride/csv_reader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace ride {

std::vector<Sample> read_samples(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::vector<Sample> samples;
    std::string line;
    std::getline(file, line);  // skip header

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string a, b, c;
        if (!std::getline(ss, a, ',') || !std::getline(ss, b, ',') ||
            !std::getline(ss, c, ',')) {
            continue;  // not enough columns
        }
        try {
            samples.push_back({std::stod(a), std::stod(b), std::stod(c)});
        } catch (const std::exception&) {
            continue;  // not numbers, skip
        }
    }
    return samples;
}

}  // namespace ride