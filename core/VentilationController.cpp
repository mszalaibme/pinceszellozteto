#include "VentilationController.h"
#include "Psychrometrics.h"
#include <limits>
namespace ventilation {
VentilationController::VentilationController(Configuration c) : config_(c.isValid() ? c : Configuration{}) {}
bool VentilationController::setConfiguration(const Configuration& c) {
    if (!c.isValid()) return false;
    config_ = c;
    return true;
}
ControllerOutput VentilationController::update(const ClimateReading& b, const ClimateReading& o, Timestamp now) {
    ControllerOutput out;
    const bool sensorsOK = b.isFresh(now, config_.sensorTimeout) && o.isFresh(now, config_.sensorTimeout);
    out.sensorWarning = !sensorsOK;
    out.highHumidityWarning = b.isFresh(now, config_.sensorTimeout) && b.humidityRH >= config_.emergencyRH;
    out.differenceGm3 = sensorsOK ? psychrometrics::absoluteHumidityGm3(b.temperatureC, b.humidityRH)
        - psychrometrics::absoluteHumidityGm3(o.temperatureC, o.humidityRH)
        : std::numeric_limits<double>::quiet_NaN();
    auto finish = [&](bool fan, ControllerState state, const char* reason) {
        if (fan != fan_) { fan_ = fan; changedAt_ = now; }
        out.fanRequested = fan_; out.state = state; out.reason = reason;
        out.dehumidificationSuggested = out.highHumidityWarning && !fan_;
        lastNow_ = now;
        return out;
    };
    if (!initialized_) { initialized_ = true; changedAt_ = now; }
    if (now < lastNow_) {
        changedAt_ = now;
        return finish(false, ControllerState::ClockError, "Monotonic clock moved backwards");
    }
    if (mode_ == Mode::Off) return finish(false, ControllerState::ManualOff, "Manual OFF");
    if (mode_ == Mode::On) return finish(true, ControllerState::ManualOn,
        sensorsOK ? "Manual ON: automatic climate protections bypassed" : "Manual ON: SENSOR ERROR");
    if (!sensorsOK) return finish(false, ControllerState::SensorError, "Sensor invalid, missing or stale");
    if (b.temperatureC <= config_.minBasementTemperature)
        return finish(false, ControllerState::BasementTooCold, "Basement too cold");
    // At approximately equal total pressure, vapor pressure orders mixing ratio.
    // Comparing g/m3 at different temperatures alone can admit wetter warm air.
    // Keep the configured AH hysteresis, but never permit actual humidification.
    if (psychrometrics::vaporPressureHpa(o.temperatureC, o.humidityRH)
        >= psychrometrics::vaporPressureHpa(b.temperatureC, b.humidityRH))
        return finish(false, ControllerState::OutdoorAirTooHumid, out.highHumidityWarning
            ? "HIGH HUMIDITY - OUTDOOR AIR TOO WET" : "Outdoor vapor pressure is not lower");
    if (out.differenceGm3 <= config_.stopVentilationHumidityAdvantage)
        return finish(false, ControllerState::OutdoorAirTooHumid, out.highHumidityWarning
            ? "HIGH HUMIDITY - OUTDOOR AIR TOO WET" : "Outdoor air too humid");
    const Timestamp elapsed = now - changedAt_;
    if (fan_) {
        if (b.humidityRH <= config_.targetBasementRH - config_.rhStartHysteresis) {
            if (elapsed < config_.ventilationMinimumOnTime)
                return finish(true, ControllerState::MinimumOnTimer, "Minimum ON timer");
            return finish(false, ControllerState::HumidityOK, "Humidity OK");
        }
        return finish(true, ControllerState::Drying, "Outdoor air is drier");
    }
    if (b.humidityRH <= config_.targetBasementRH)
        return finish(false, ControllerState::HumidityOK, "Humidity OK");
    if (out.differenceGm3 < config_.minVentilationHumidityAdvantage)
        return finish(false, ControllerState::OutdoorAirTooHumid, out.highHumidityWarning
            ? "HIGH HUMIDITY - INSUFFICIENT DRYING ADVANTAGE" : "Insufficient drying advantage");
    if (elapsed < config_.ventilationMinimumOffTime)
        return finish(false, ControllerState::MinimumOffTimer, "Minimum OFF timer");
    return finish(true, ControllerState::Drying, "Outdoor air is drier");
}
}
