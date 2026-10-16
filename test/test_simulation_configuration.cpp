#include "simulator/BatchSimulation.h"
#include "Test.h"
#include <fstream>
using namespace ventilation::simulation;
int main() { return run([] {
    const auto root=std::filesystem::path(__FILE__).parent_path().parent_path();
    const auto c=loadSimulationConfiguration(root/"config/baseline.ini");
    CHECK(c.isValid()); NEAR(c.basementVolumeM3,26.4,0);
    NEAR(c.dehumidifier.referenceExtractionLh,0.21,0);
    NEAR(c.dehumidifier.extractionFactor(15,80),1,1e-12);
    NEAR(c.dehumidifier.powerWatts,300,0);
    CHECK(!c.thermal.includeDehumidifierLatentHeat && c.tankEmptyIntervalHours==0);
    auto baseline=baselineExperiments(c); auto jobs=sensitivityExperiments(c);
    CHECK(baseline.size()==5 && jobs.size()==945);
    unsigned factorial=0,power=0,thermal=0;
    for(const auto& job:jobs) {
        CHECK(job.configuration.isValid());
        factorial+=job.group=="factorial";
        power+=job.group=="power-wet" || job.group=="power-baseline";
        thermal+=job.group=="thermal";
    }
    CHECK(factorial==900 && power==10 && thermal==27);
    auto invalid=c; invalid.sweeps.airflows={-1}; CHECK(!invalid.isValid());
    invalid=c; invalid.thermal.effectiveThermalCapacityJPerK=NAN; CHECK(!invalid.isValid());
    const auto temp=std::filesystem::current_path()/"configuration-regression.ini";
    auto rejects=[&](const std::string& content) {
        { std::ofstream out(temp); out<<content; }
        bool failed=false; try { loadSimulationConfiguration(temp); } catch(const std::exception&) { failed=true; }
        CHECK(failed);
    };
    rejects("typo=26.4\n"); rejects("basementVolumeM3=26.4\nbasementVolumeM3=26.4\n");
    rejects("referenceRH=35\n"); rejects("dehumidifierPowerW=NaN\n"); rejects("tankEstimationEnabled=maybe\n");
    rejects("sweepAirflowsM3h=60,\n");
    std::filesystem::remove(temp);
    SimulationOptions options; options.hours=1; options.configuration=c;
    const auto retained=runScenario(baselineScenario(c),options);
    options.retainRows=false; const auto compact=runScenario(baselineScenario(c),options);
    CHECK(compact.rows.empty() && compact.sampleCount==retained.rows.size());
    NEAR(retained.averageBasementRH,compact.averageBasementRH,1e-12);
    NEAR(compact.massBalanceErrorGram,0,1e-6); NEAR(compact.thermalBalanceErrorJ,0,0.01);
}); }
