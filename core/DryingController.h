#pragma once
#include "Configuration.h"
#include "ControllerState.h"
#include "DehumidifierState.h"
namespace ventilation {
class DryingController {
public:
    explicit DryingController(Configuration config = {});
    bool setConfiguration(const Configuration& config);
    const Configuration& configuration() const { return config_; }
    void setMode(Mode mode) { mode_ = mode; }
    Mode mode() const { return mode_; }
    void resetTankEstimate() { estimateLiters_=0; }
    ControllerOutput update(const ClimateReading& basement, const ClimateReading& outside,
        const DehumidifierReading& feedback, Timestamp now, const TankReading& tank = {});
private:
    Configuration config_;
    Mode mode_ = Mode::Auto;
    bool initialized_=false, fan_=false, dehum_=false, demand_=false;
    bool monitorWindow_=false, ineffective_=false, feedbackRunning_=false, noDrawTiming_=false;
    bool ranSinceCommand_=false;
    Timestamp lastNow_=0, fanChanged_=0, dehumChanged_=0, performanceAt_=0, noDrawAt_=0;
    double baselineAH_=0, estimateLiters_=0, runtimeSeconds_=0;
};
}
