#pragma once
#include <cstdint>
#include <cmath>
namespace ventilation {
using Timestamp = std::uint64_t; // Monotonic milliseconds since boot; not wall time.
struct ClimateReading {
    double temperatureC = 0;
    double humidityRH = 0;
    Timestamp measuredAtMs = 0;
    bool valid = false;
    bool isPlausible() const {
        return valid && std::isfinite(temperatureC) && std::isfinite(humidityRH)
            && temperatureC >= -20 && temperatureC <= 50
            && humidityRH >= 0 && humidityRH <= 100;
    }
    bool isFresh(Timestamp now, Timestamp timeout) const {
        return isPlausible() && measuredAtMs <= now && now - measuredAtMs < timeout;
    }
};
}
