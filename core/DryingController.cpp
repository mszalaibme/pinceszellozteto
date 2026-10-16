#include "DryingController.h"
#include "Psychrometrics.h"
#include <algorithm>
#include <limits>
namespace ventilation {
DryingController::DryingController(Configuration c) : config_(c.isValid()?c:Configuration{}) {}
bool DryingController::setConfiguration(const Configuration& c) {
    if(!c.isValid()) return false;
    config_=c; return true;
}
ControllerOutput DryingController::update(const ClimateReading& b,const ClimateReading& o,
    const DehumidifierReading& feedback,Timestamp now,const TankReading& tank) {
    ControllerOutput out;
    const bool valid=b.isFresh(now,config_.sensorTimeout) && o.isFresh(now,config_.sensorTimeout);
    const bool monitored=feedback.fresh(now,config_.dehumidifierMonitorTimeout)
        && feedback.feedback!=DehumidifierFeedback::Unknown;
    const bool powerKnown=monitored && std::isfinite(feedback.powerWatts) && feedback.powerWatts>=0;
    const bool running=monitored && feedback.feedback==DehumidifierFeedback::Running
        && (!powerKnown || feedback.powerWatts>=config_.dehumidifierMinimumPowerWatts);
    out.sensorWarning=!valid;
    out.feedbackUnavailable=!monitored;
    out.highHumidityWarning=b.isFresh(now,config_.sensorTimeout) && b.humidityRH>=config_.emergencyRH;
    out.differenceGm3=valid?psychrometrics::absoluteHumidityGm3(b.temperatureC,b.humidityRH)
        -psychrometrics::absoluteHumidityGm3(o.temperatureC,o.humidityRH):std::numeric_limits<double>::quiet_NaN();
    if(!initialized_) { initialized_=true; lastNow_=fanChanged_=dehumChanged_=now; }
    const bool backwards=now<lastNow_;
    if(!backwards && dehum_) {
        const double dt=static_cast<double>(now-lastNow_)/1000;
        runtimeSeconds_+=dt;
        if(config_.tankEstimationEnabled && running && feedbackRunning_)
            estimateLiters_=std::min(config_.tankCapacityLiters,estimateLiters_+dt/3600*config_.estimatedExtractionLitersPerHour);
    }
    auto finish=[&](bool fan,bool dehum,ControllerState state,ReasonCode reason,const char* message) {
        if(fan!=fan_) { fan_=fan; fanChanged_=now; }
        if(dehum!=dehum_) {
            dehum_=dehum; dehumChanged_=now; monitorWindow_=false; ineffective_=false; noDrawTiming_=false; ranSinceCommand_=false;
        }
        if(!dehum_) { monitorWindow_=false; ineffective_=false; noDrawTiming_=false; }
        feedbackRunning_=running && dehum_; lastNow_=now;
        out.fanRequested=out.ventilationRequested=fan_;
        out.dehumidifierRequested=dehum_;
        out.state=state; out.reasonCode=reason; out.reason=message;
        out.dehumidifierIneffective=ineffective_;
        out.estimatedTankLevelLiters=estimateLiters_; out.tankEstimateAvailable=config_.tankEstimationEnabled;
        out.dehumidifierRuntimeSeconds=runtimeSeconds_;
        return out;
    };
    if(backwards) {
        fanChanged_=dehumChanged_=now; demand_=false;
        return finish(false,false,ControllerState::ClockError,ReasonCode::ClockFailure,"CLOCK_ERROR");
    }
    if(mode_==Mode::Off) { demand_=false; return finish(false,false,ControllerState::ManualOff,ReasonCode::Manual,"MANUAL_OFF"); }
    if(mode_==Mode::Auto && !valid) {
        demand_=false;
        return finish(false,false,ControllerState::SensorError,ReasonCode::SensorFailure,"SENSOR_ERROR");
    }
    const bool cold=valid && b.temperatureC<=config_.minBasementTemperature;
    const bool actuallyDrier=valid && psychrometrics::vaporPressureHpa(o.temperatureC,o.humidityRH)
        <psychrometrics::vaporPressureHpa(b.temperatureC,b.humidityRH);
    const bool outdoorSuitable=valid && !cold && actuallyDrier
        && (fan_ ? out.differenceGm3>config_.stopVentilationHumidityAdvantage
                 : out.differenceGm3>=config_.minVentilationHumidityAdvantage);
    const bool manualVent=mode_==Mode::On;
    const bool manualDehum=mode_==Mode::ManualDehumidifier;
    const bool tankFault=tank.enabled && !tank.fresh(now,config_.sensorTimeout);
    const bool tankBlocked=tank.enabled && (tank.full || tankFault);
    if(mode_==Mode::Auto) {
        if(b.humidityRH>=config_.targetBasementRH+config_.rhStartHysteresis) demand_=true;
        if(b.humidityRH<=config_.targetBasementRH) demand_=false;
    }
    bool wantVent=manualVent || (mode_==Mode::Auto && demand_ && outdoorSuitable);
    bool wantDehum=manualDehum || (mode_==Mode::Auto && demand_ && !outdoorSuitable);
    // Safety overrides minimum ON; ordinary RH stop and strategy changes honor it.
    if(fan_ && !manualDehum && !manualVent && outdoorSuitable && now-fanChanged_<config_.ventilationMinimumOnTime)
        wantVent=true;
    const bool deviceFault=monitored && feedback.feedback==DehumidifierFeedback::Fault;
    if(dehum_ && !manualVent && !deviceFault && !tankBlocked && now-dehumChanged_<config_.dehumidifierMinimumOnTime) {
        wantDehum=true; wantVent=false;
    }
    if(wantVent) {
        if(!fan_ && mode_==Mode::Auto && now-fanChanged_<config_.ventilationMinimumOffTime) {
            out.remainingWaitMs=config_.ventilationMinimumOffTime-(now-fanChanged_);
            return finish(false,false,ControllerState::WaitingMinimumOffTime,ReasonCode::MinimumOff,"VENTILATION_MINIMUM_OFF");
        }
        const bool holding=!demand_ && mode_==Mode::Auto;
        return finish(true,false,manualVent?ControllerState::ManualVentilation:ControllerState::Ventilating,
            holding?ReasonCode::MinimumOn:manualVent?ReasonCode::Manual:ReasonCode::OutdoorAirDrier,
            holding?"VENTILATION_MINIMUM_ON":manualVent?"MANUAL_VENTILATION":"OUTDOOR_AIR_DRYER");
    }
    if(!wantDehum)
        return finish(false,false,ControllerState::Idle,ReasonCode::HumidityOK,"HUMIDITY_OK");
    if(tankBlocked)
        return finish(false,false,ControllerState::DehumidifierError,
            tankFault?ReasonCode::TankSensorFailure:ReasonCode::TankFull,
            tankFault?"TANK_SENSOR_FAILURE":"TANK_FULL");
    if(!config_.dehumidifierAutoRestartSupported)
        return finish(false,false,ControllerState::DehumidifierError,ReasonCode::AutoRestartUnverified,"AUTO_RESTART_NOT_VERIFIED");
    if(deviceFault)
        return finish(false,false,ControllerState::DehumidifierError,ReasonCode::DeviceFault,"DEHUMIDIFIER_FAULT");
    if(!dehum_ && now-dehumChanged_<config_.dehumidifierMinimumOffTime) {
        out.remainingWaitMs=config_.dehumidifierMinimumOffTime-(now-dehumChanged_);
        return finish(false,false,ControllerState::WaitingMinimumOffTime,ReasonCode::MinimumOff,"COMPRESSOR_MINIMUM_OFF");
    }
    const bool noDraw=monitored && (feedback.feedback==DehumidifierFeedback::NoPowerDraw
        || feedback.feedback==DehumidifierFeedback::Off || (powerKnown && feedback.powerWatts<config_.dehumidifierMinimumPowerWatts));
    if(dehum_ && running) ranSinceCommand_=true;
    if(dehum_ && noDraw) {
        if(!noDrawTiming_) { noDrawTiming_=true; noDrawAt_=now; }
    } else noDrawTiming_=false;
    // Startup grace is measured from the socket command, later loss from its first observation.
    const bool noDrawError=dehum_ && noDraw && now-dehumChanged_>=config_.dehumidifierStartupGracePeriod
        && (!ranSinceCommand_ || now-noDrawAt_>=config_.dehumidifierStartupGracePeriod);
    if(config_.performanceMonitorEnabled && dehum_ && running && valid) {
        const double ah=psychrometrics::absoluteHumidityGm3(b.temperatureC,b.humidityRH);
        if(!monitorWindow_) { monitorWindow_=true; performanceAt_=now; baselineAH_=ah; }
        if(now-performanceAt_>=config_.dehumidifierIneffectiveTimeout) {
            ineffective_=baselineAH_-ah<config_.minimumAHImprovementGm3;
            performanceAt_=now; baselineAH_=ah;
        }
    } else monitorWindow_=false;
    if(noDrawError)
        return finish(false,true,ControllerState::DehumidifierError,ReasonCode::NoPowerDraw,"DEHUMIDIFIER_NO_POWER_DRAW: CHECK TANK / APPLIANCE");
    return finish(false,true,manualDehum?ControllerState::ManualDehumidifier:ControllerState::Dehumidifying,
        manualDehum?ReasonCode::Manual:ReasonCode::DehumidifierPreferred,
        ineffective_?"DEHUMIDIFIER_INEFFECTIVE":!monitored?"DEHUMIDIFIER_FEEDBACK_UNKNOWN":"DEHUMIDIFIER_PREFERRED");
}
}
