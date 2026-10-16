#pragma once
#include "core/ClimateData.h"
namespace ventilation::simulation {
struct ModelParameters {
    double volumeM3 = 26;
    double fanAirflowM3PerHour = 100; // Volume flow expressed at basement conditions.
    double moistureGenerationGramPerHour = 25;
};
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
        double seconds, double wallTemperatureC, double heatCapacityJPerK=3000000, double wallConductanceWPerK=35);
    double temperatureC() const { return temperatureC_; }
    double vaporMassGram() const { return vaporMassGram_; }
    double absoluteHumidityGm3() const { return vaporMassGram_ / parameters_.volumeM3; }
    double relativeHumidity() const;
    ClimateReading reading(Timestamp now) const;
    const WaterBalance& balance() const { return balance_; }
private:
    ModelParameters parameters_;
    double temperatureC_;
    double vaporMassGram_;
    WaterBalance balance_;
    double saturationMassGram() const;
};
}
