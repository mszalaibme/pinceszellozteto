#include "DehumidifierModel.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace ventilation::simulation {
DehumidifierModel::DehumidifierModel(DehumidifierParameters p) : parameters_(p),tankLiters_(p.initialTankLevelLiters) {
    for(double x:{p.ratedExtractionLitersPerDay,p.tankCapacityLiters,p.initialTankLevelLiters,p.powerWatts,
        p.standbyPowerWatts,p.minimumOnTime,p.minimumOffTime,p.startupDelay,p.performanceFactor})
        if(!std::isfinite(x) || x<0) throw std::invalid_argument("Invalid dehumidifier parameter");
    if(p.tankCapacityLiters<=0 || p.initialTankLevelLiters>p.tankCapacityLiters
        || p.minimumOffTime>86400 || p.startupDelay>86400 || p.minimumOnTime>86400)
        throw std::invalid_argument("Invalid tank or timer");
}
void DehumidifierModel::stop(double at) {
    if(running_) {
        if(at-runStartedAt_<parameters_.minimumOnTime) ++shortRuns_;
        stoppedAt_=at; running_=false;
    }
}
void DehumidifierModel::emptyTank() {
    tankLiters_=0;
    commandOnAt_=now_; // A fresh appliance startup delay after the local tank event.
}
void DehumidifierModel::step(bool command,BasementModel& b,double seconds) {
    if(!std::isfinite(seconds) || seconds<=0 || seconds>86400) throw std::invalid_argument("Invalid appliance step");
    if(command!=socket_) {
        if(command) { commandOnAt_=now_; standby_=!parameters_.autoRestartAfterPowerRestore; }
        else { stop(now_); standby_=false; }
        socket_=command;
    }
    rate_=parameters_.ratedExtractionLitersPerDay/24
        *std::clamp((b.relativeHumidity()-35)/45,0.0,1.2)
        *std::clamp((b.temperatureC()-5)/15,0.0,1.3)*parameters_.performanceFactor;
    if(!socket_ || standby_ || tankFull()) { stop(now_); now_+=seconds; return; }
    const double ready=running_?now_:std::max(commandOnAt_+parameters_.startupDelay,stoppedAt_+parameters_.minimumOffTime);
    if(ready>=now_+seconds) { now_+=seconds; return; }
    const double runFrom=std::max(now_,ready);
    if(!running_) { running_=true; runStartedAt_=runFrom; }
    double runSeconds=now_+seconds-runFrom;
    if(rate_>0) runSeconds=std::min(runSeconds,(parameters_.tankCapacityLiters-tankLiters_)/rate_*3600);
    const double removed=b.removeWaterGram(rate_*runSeconds/3600*1000)/1000;
    tankLiters_=std::min(parameters_.tankCapacityLiters,tankLiters_+removed);
    totalExtracted_+=removed; runtime_+=runSeconds;
    if(tankFull()) stop(runFrom+runSeconds);
    now_+=seconds;
}
DehumidifierReading DehumidifierModel::feedback(Timestamp now) const {
    DehumidifierFeedback f=DehumidifierFeedback::Off;
    if(socket_) {
        if(running_) f=DehumidifierFeedback::Running;
        else if(tankFull() || standby_) f=DehumidifierFeedback::NoPowerDraw;
        else f=DehumidifierFeedback::Starting;
    }
    return {f,powerWatts(),now,true};
}
}
