#pragma once
#include "SimulationConfiguration.h"
#include <limits>
#include <optional>
#include <vector>
namespace ventilation::simulation {
struct CalibrationMeasurements {
    double measuredWaterCollectedLiters=std::numeric_limits<double>::quiet_NaN();
    double measurementDurationHours=std::numeric_limits<double>::quiet_NaN();
    std::optional<double> measuredBasementTemperature,measuredBasementRH,measuredOutdoorTemperature,measuredOutdoorRH;
    std::optional<double> fanRuntimeHours,dehumidifierRuntimeHours;
    std::optional<double> initialBasementTemperature,initialBasementRH,finalBasementTemperature,finalBasementRH;
    std::optional<double> measuredCondensateLiters;
    bool assumeSteadyState=false,assumeNoCondensation=false;
};
struct CalibrationResult {
    double observedAverageCollectionLh=0,effectiveExtractionLowerBoundLh=0;
    std::optional<double> effectiveDehumidifierExtractionLh,fittedReferenceExtractionLh;
    std::optional<double> moistureIngressGPerHour,moistureIngressLowerBoundGPerHour;
    std::vector<std::string> assumptions,warnings;
};
CalibrationMeasurements loadCalibrationMeasurements(const std::filesystem::path& path);
CalibrationResult calibrate(const CalibrationMeasurements& measurements,const SimulationConfiguration& configuration = {});
void writeCalibration(const CalibrationResult& result,const std::filesystem::path& folder);
}
