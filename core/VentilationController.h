#pragma once
#include "ClimateData.h"
#include "Configuration.h"
#include "ControllerState.h"
namespace ventilation {
class VentilationController {
public:
    explicit VentilationController(Configuration config = {});
    bool setConfiguration(const Configuration& config);
    const Configuration& configuration() const { return config_; }
    void setMode(Mode mode) { mode_ = mode; }
    Mode mode() const { return mode_; }
    ControllerOutput update(const ClimateReading& basement, const ClimateReading& outside, Timestamp now);
private:
    Configuration config_;
    Mode mode_ = Mode::Auto;
    bool fan_ = false;
    bool initialized_ = false;
    Timestamp changedAt_ = 0;
    Timestamp lastNow_ = 0;
};
}
