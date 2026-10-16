#pragma once
#include "core/DryingController.h"
#include "DehumidifierModel.h"
#include "SimulationConfiguration.h"
#include <string>
#include <vector>
#include <filesystem>
#include <optional>
namespace ventilation::simulation {
struct Scenario {
    std::string id, title, description;
    double basementT=17, basementRH=82, outdoorT=27, outdoorRH=65, sourceGramPerHour=25;
    double initialTankLiters=0, performanceFactor=1;
    bool autoRestart=true, dailyWeather=false, rapidWeather=false;
    bool baselineWeather=false;
    Mode mode=Mode::Auto;
    double emptyTankAtSeconds=-1, sensorFailureAtSeconds=-1;
};
struct SimulationOptions {
    double hours=24, stepSeconds=60, sourceOverride=-1;
    std::optional<SimulationConfiguration> configuration;
    bool retainRows=true;
};
struct SimulationRow {
    Timestamp timestamp=0;
    ClimateReading basement, outside;
    ControllerOutput output;
    DehumidifierReading feedback;
    double tankLiters=0, powerWatts=0;
    bool tankFull=false, compressorRunning=false;
    double intervalSeconds=0, ventilationWaterRemovalGPerHour=0, dehumidifierWaterRemovalGPerHour=0;
    double cumulativeVentilationWaterRemovedL=0,cumulativeDehumidifierWaterRemovedL=0,cumulativeEnergyKWh=0;
    double averageDehumidifierPowerW=0,averageVentilationPowerW=0;
};
struct SimulationResult {
    Scenario scenario;
    std::vector<SimulationRow> rows;
    double fanHours=0, dehumCommandHours=0, compressorHours=0, extractedLiters=0;
    double generatedLiters=0, netVentilationLiters=0, condensedLiters=0, massBalanceErrorGram=0;
    double firstTankFullSeconds=-1;
    SimulationConfiguration configuration;
    double hours=0, stepSeconds=0,averageBasementRH=0,maximumBasementRH=0,minimumBasementTemperature=0;
    double timeAbove70RH=0,timeAbove80RH=0,timeBelowMinimumTemperature=0;
    double cumulativeEnergyKWh=0,dehumidifierEnergyKWh=0,ventilationEnergyKWh=0,thermalBalanceErrorJ=0;
    double emptiedTankLiters=0;
    double vaporStorageChangeGram=0,finalTankLevelLiters=0;
    unsigned tankFullEventCount=0;
    std::size_t sampleCount=0;
};
std::vector<Scenario> scenarios();
Scenario baselineScenario(const SimulationConfiguration& configuration = {});
Scenario calibrationObservationScenario();
SimulationResult runScenario(const Scenario& scenario, SimulationOptions options = {});
void writeCsv(const SimulationResult& result,const std::filesystem::path& path);
void writeDashboard(const std::vector<SimulationResult>& results,const std::filesystem::path& path);
}
