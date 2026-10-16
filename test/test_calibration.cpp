#include "simulator/Calibration.h"
#include "Test.h"
using namespace ventilation::simulation;
int main() { return run([] {
    CalibrationMeasurements m; m.measuredWaterCollectedLiters=5; m.measurementDurationHours=24;
    auto r=calibrate(m);
    NEAR(r.observedAverageCollectionLh,5.0/24,1e-12);
    CHECK(!r.moistureIngressGPerHour && !r.effectiveDehumidifierExtractionLh && !r.fittedReferenceExtractionLh);
    m.fanRuntimeHours=0; m.dehumidifierRuntimeHours=24;
    m.measuredBasementTemperature=15; m.measuredBasementRH=80;
    m.assumeSteadyState=true; m.assumeNoCondensation=true;
    r=calibrate(m);
    NEAR(*r.moistureIngressGPerHour,5000.0/24,1e-9);
    NEAR(*r.effectiveDehumidifierExtractionLh,5.0/24,1e-12);
    NEAR(*r.fittedReferenceExtractionLh,5.0/24,1e-12);
    m.dehumidifierRuntimeHours=12;
    NEAR(*calibrate(m).effectiveDehumidifierExtractionLh,5.0/12,1e-12);
    m.assumeNoCondensation=false;
    r=calibrate(m); CHECK(!r.moistureIngressGPerHour && r.moistureIngressLowerBoundGPerHour.has_value());
    m.measuredCondensateLiters=1;
    NEAR(*calibrate(m).moistureIngressGPerHour,6000.0/24,1e-9);
    m.assumeSteadyState=false;
    m.initialBasementTemperature=15; m.initialBasementRH=80;
    m.finalBasementTemperature=15; m.finalBasementRH=80;
    CHECK(calibrate(m).moistureIngressGPerHour.has_value());
    m.dehumidifierRuntimeHours=0;
    bool failed=false; try { calibrate(m); } catch(const std::invalid_argument&) { failed=true; }
    CHECK(failed);
}); }
