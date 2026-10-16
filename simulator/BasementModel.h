#pragma once
#include "core/ClimateData.h"
#include "ModelConfiguration.h"
namespace ventilation::simulation {
struct ThermalBalance { double suppliedHeatJ=0,wallHeatLossJ=0,ventilationHeatLossJ=0,sensibleChangeJ=0; };
struct WaterBalance {
    double initialVaporGram = 0;
    double generatedGram = 0;
    double incomingGram = 0;
    double outgoingGram = 0;
    double condensedGram = 0; // Permanently removed; no re-evaporation model.
    double extractedGram = 0; // Actual dehumidifier removal into its tank.
};
class BasementModel {
public:
    BasementModel(ModelParameters parameters = {}, double temperatureC = 14, double initialRH = 80);
    // Temperature is a prescribed boundary condition, not an RH interpolation.
    void setTemperatureC(double temperatureC);
    void step(const ClimateReading& outside, bool fanOn, double durationSeconds);
    double removeWaterGram(double requestedGram);
    void evolveTemperature(const ClimateReading& outside, bool fanOn, double electricalHeatWatts,
        double seconds, const ThermalParameters& parameters);
    double temperatureC() const { return temperatureC_; }
    double vaporMassGram() const { return vaporMassGram_; }
    double volumeM3() const { return parameters_.volumeM3; }
    double absoluteHumidityGm3() const { return vaporMassGram_ / parameters_.volumeM3; }
    double relativeHumidity() const;
    ClimateReading reading(Timestamp now) const;
    const WaterBalance& balance() const { return balance_; }
    const ThermalBalance& thermalBalance() const { return thermalBalance_; }
private:
    ModelParameters parameters_;
    double temperatureC_;
    double vaporMassGram_;
    WaterBalance balance_;
    ThermalBalance thermalBalance_;
    double saturationMassGram() const;
};
}
