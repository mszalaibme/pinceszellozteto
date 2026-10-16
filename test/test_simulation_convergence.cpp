#include "simulator/SimulationRunner.h"
#include "core/Psychrometrics.h"
#include "Test.h"
using namespace ventilation::simulation;
int main() { return run([] {
    // Reproducer: a one-hour constant-RH extraction step used to over-dry below the curve's cutoff.
    DehumidifierParameters p; p.minimumOffTime=0; p.startupDelay=0;
    BasementModel once({26.4,0,0},15,80),split({26.4,0,0},15,80);
    DehumidifierModel a(p),b(p);
    a.step(true,once,3600);
    for(int i=0;i<3600;++i) b.step(true,split,1);
    CHECK(once.relativeHumidity()>=p.minimumExtractionRH);
    NEAR(once.vaporMassGram(),split.vaporMassGram(),1e-8);
    NEAR(a.totalExtractedLiters(),b.totalExtractedLiters(),1e-10);
    // Exporting twice as often must not change the controller's polling or the physics.
    SimulationConfiguration c;
    SimulationOptions options; options.configuration=c; options.hours=6; options.retainRows=false;
    auto baseline=baselineScenario(c);
    const auto coarse=runScenario(baseline,options);
    options.stepSeconds=30; const auto fineExport=runScenario(baseline,options);
    NEAR(coarse.averageBasementRH,fineExport.averageBasementRH,1e-10);
    NEAR(coarse.fanHours,fineExport.fanHours,1e-10);
    // Sensitive 500 W wet-outdoor case: physical refinement at fixed 60s controller cadence.
    c.dehumidifier.powerWatts=500; c.moistureIngressGPerHour=200; c.tankEmptyIntervalHours=24;
    options.configuration=c; options.hours=168; options.stepSeconds=60;
    const auto wet=scenarios().at(1);
    const auto result10=runScenario(wet,options);
    c.physicalTimeStepSeconds=5; options.configuration=c; const auto result5=runScenario(wet,options);
    c.physicalTimeStepSeconds=1; options.configuration=c; const auto result1=runScenario(wet,options);
    NEAR(result10.averageBasementRH,result1.averageBasementRH,0.002);
    NEAR(result5.averageBasementRH,result1.averageBasementRH,0.001);
    NEAR(result10.compressorHours,result1.compressorHours,1e-5);
    NEAR(result10.cumulativeEnergyKWh,result1.cumulativeEnergyKWh,1e-6);
    NEAR(result10.condensedLiters,result1.condensedLiters,0.001);
    CHECK(result1.maximumBasementRH>99); // A remaining control-policy limit, not erased by the model.
}); }
