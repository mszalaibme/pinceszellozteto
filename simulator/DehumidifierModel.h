#pragma once
#include "BasementModel.h"
#include "core/DehumidifierState.h"
namespace ventilation::simulation {
class DehumidifierModel {
public:
    explicit DehumidifierModel(DehumidifierParameters p = {});
    void step(bool socketOn, BasementModel& basement, double seconds);
    void emptyTank();
    DehumidifierReading feedback(Timestamp now) const;
    double tankLevelLiters() const { return tankLiters_; }
    bool tankFull() const { return tankLiters_>=parameters_.tankCapacityLiters-1e-12; }
    bool compressorRunning() const { return running_; }
    double powerWatts() const { return socket_ ? (running_?parameters_.powerWatts:parameters_.standbyPowerWatts):0; }
    double effectiveExtractionLitersPerHour() const { return rate_; }
    double totalExtractedLiters() const { return totalExtracted_; }
    double runtimeSeconds() const { return runtime_; }
    unsigned interruptedShortRuns() const { return shortRuns_; }
    double intervalEnergyWh() const { return intervalEnergyWh_; }
    unsigned tankFullEvents() const { return fullEvents_; }
    double firstTankFullSeconds() const { return firstFullAt_; }
    double emptiedLiters() const { return emptiedLiters_; }
private:
    DehumidifierParameters parameters_;
    double now_=0, commandOnAt_=0, stoppedAt_=0, runStartedAt_=0;
    double tankLiters_=0, rate_=0, totalExtracted_=0, runtime_=0;
    bool socket_=false, running_=false, standby_=false;
    unsigned shortRuns_=0;
    unsigned fullEvents_=0;
    double intervalEnergyWh_=0,firstFullAt_=-1,emptiedLiters_=0;
    void stop(double at);
};
}
