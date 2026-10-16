#pragma once
#include "core/DryingController.h"
#include "DehumidifierModel.h"
#include <string>
#include <vector>
#include <filesystem>
namespace ventilation::simulation {
struct Scenario {
    std::string id, title, description;
    double basementT=17, basementRH=82, outdoorT=27, outdoorRH=65, sourceGramPerHour=25;
    double initialTankLiters=0, performanceFactor=1;
    bool autoRestart=true, dailyWeather=false, rapidWeather=false;
    double emptyTankAtSeconds=-1, sensorFailureAtSeconds=-1;
};
struct SimulationOptions { double hours=24, stepSeconds=60, sourceOverride=-1; };
struct SimulationRow {
    Timestamp timestamp=0;
    ClimateReading basement, outside;
    ControllerOutput output;
    DehumidifierReading feedback;
    double tankLiters=0, powerWatts=0;
    bool tankFull=false, compressorRunning=false;
};
struct SimulationResult {
    Scenario scenario;
    std::vector<SimulationRow> rows;
    double fanHours=0, dehumCommandHours=0, compressorHours=0, extractedLiters=0;
    double generatedLiters=0, netVentilationLiters=0, condensedLiters=0, massBalanceErrorGram=0;
    double firstTankFullSeconds=-1;
};
std::vector<Scenario> scenarios();
SimulationResult runScenario(const Scenario& scenario, SimulationOptions options = {});
void writeCsv(const SimulationResult& result,const std::filesystem::path& path);
void writeDashboard(const std::vector<SimulationResult>& results,const std::filesystem::path& path);
}
