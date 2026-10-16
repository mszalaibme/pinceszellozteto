#pragma once
namespace ventilation::simulation {
struct ModelParameters {
    double volumeM3=26.4;
    double fanAirflowM3PerHour=100;
    double moistureGenerationGramPerHour=100;
};
struct ThermalParameters {
    // SIMULATION ASSUMPTION: effective BUILDING capacity; air capacity is added separately.
    double effectiveThermalCapacityJPerK=3000000;
    double wallTemperatureC=14;
    double wallAirHeatTransferWPerK=35;
    double airDensityKgM3=1.2;
    double airSpecificHeatJPerKgK=1005;
    bool includeDehumidifierLatentHeat=false;
    double latentHeatJPerKg=2450000;
};
struct DehumidifierParameters {
    // Empirical reference, NOT a factory-rated extraction specification.
    double referenceExtractionLh=0.21;
    double referenceTemperatureC=15;
    double referenceRH=80;
    double minimumExtractionTemperatureC=5;
    double minimumExtractionRH=35;
    double temperatureExponent=1;
    double humidityExponent=1.5;
    double maximumTemperatureFactor=1.5;
    double maximumHumidityFactor=1.5;
    double tankCapacityLiters=5;
    double initialTankLevelLiters=0;
    double powerWatts=300; // SIMULATION ASSUMPTION, NEEDS HARDWARE VERIFICATION.
    double standbyPowerWatts=2; // SIMULATION ASSUMPTION.
    double minimumOnTime=900;
    double minimumOffTime=600;
    double startupDelay=30;
    bool autoRestartAfterPowerRestore=true; // Simulated capability, not verified real hardware.
    double performanceFactor=1;
    double extractionFactor(double temperatureC,double humidityRH) const;
    bool isValid() const;
};
struct WeatherParameters {
    double minimumTemperatureC=4;
    double maximumTemperatureC=12;
    double nightRH=95;
    double dayRH=75;
    double peakTemperatureHour=14;
    double periodHours=24;
};
}
