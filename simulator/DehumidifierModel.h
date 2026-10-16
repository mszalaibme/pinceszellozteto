#pragma once
#include "BasementModel.h"
#include "core/DehumidifierState.h"
namespace ventilation::simulation {
struct DehumidifierParameters {
    double ratedExtractionLitersPerDay=10;
    double tankCapacityLiters=5;
    double initialTankLevelLiters=0;
    double powerWatts=310;
    double standbyPowerWatts=2;
    double minimumOnTime=900; // seconds; used to report interrupted short runs.
    double minimumOffTime=600;
    double startupDelay=30;
    bool autoRestartAfterPowerRestore=true;
    double performanceFactor=1;
};
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
private:
    DehumidifierParameters parameters_;
    double now_=0, commandOnAt_=0, stoppedAt_=0, runStartedAt_=0;
    double tankLiters_=0, rate_=0, totalExtracted_=0, runtime_=0;
    bool socket_=false, running_=false, standby_=false;
    unsigned shortRuns_=0;
    void stop(double at);
};
}
