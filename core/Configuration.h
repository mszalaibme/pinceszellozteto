#pragma once
#include "ClimateData.h"
#include <cmath>
namespace ventilation {
struct Configuration {
    double targetBasementRH = 65;
    double emergencyRH = 80;
    double minVentilationHumidityAdvantage = 1.0;
    double stopVentilationHumidityAdvantage = 0.4;
    double minBasementTemperature = 10;
    double rhStartHysteresis = 3;
    Timestamp ventilationMinimumOnTime = 600000;
    Timestamp ventilationMinimumOffTime = 300000;
    Timestamp sensorTimeout = 120000;
    Timestamp dehumidifierMinimumOnTime = 900000;
    Timestamp dehumidifierMinimumOffTime = 600000;
    Timestamp dehumidifierStartupGracePeriod = 120000;
    Timestamp dehumidifierMonitorTimeout = 120000;
    bool dehumidifierAutoRestartSupported = false; // Must be verified on the actual appliance.
    double dehumidifierMinimumPowerWatts = 30;
    double tankCapacityLiters = 5;
    bool tankEstimationEnabled = false;
    double estimatedExtractionLitersPerHour = 5.0 / 24.0;
    bool performanceMonitorEnabled = true;
    Timestamp dehumidifierIneffectiveTimeout = 10800000;
    double minimumAHImprovementGm3 = 0.2;
    bool isValid() const {
        return std::isfinite(targetBasementRH) && std::isfinite(emergencyRH)
            && std::isfinite(minVentilationHumidityAdvantage) && std::isfinite(stopVentilationHumidityAdvantage)
            && std::isfinite(minBasementTemperature) && std::isfinite(rhStartHysteresis)
            && targetBasementRH > 0 && targetBasementRH < emergencyRH && emergencyRH <= 100
            && rhStartHysteresis > 0 && rhStartHysteresis < targetBasementRH
            && stopVentilationHumidityAdvantage >= 0 && minVentilationHumidityAdvantage > stopVentilationHumidityAdvantage
            && minVentilationHumidityAdvantage <= 50
            && minBasementTemperature >= -20 && minBasementTemperature < 50
            && ventilationMinimumOnTime <= 86400000 && ventilationMinimumOffTime <= 86400000
            && sensorTimeout > 0 && sensorTimeout <= 3600000
            && targetBasementRH + rhStartHysteresis <= 100
            && dehumidifierMinimumOnTime <= 86400000 && dehumidifierMinimumOffTime <= 86400000
            && dehumidifierStartupGracePeriod > 0 && dehumidifierStartupGracePeriod <= 3600000
            && dehumidifierMonitorTimeout > 0 && dehumidifierMonitorTimeout <= 3600000
            && std::isfinite(dehumidifierMinimumPowerWatts) && dehumidifierMinimumPowerWatts > 0
            && std::isfinite(tankCapacityLiters) && tankCapacityLiters > 0 && tankCapacityLiters <= 100
            && std::isfinite(estimatedExtractionLitersPerHour) && estimatedExtractionLitersPerHour >= 0
            && estimatedExtractionLitersPerHour <= 100
            && dehumidifierIneffectiveTimeout >= 60000 && dehumidifierIneffectiveTimeout <= 604800000
            && std::isfinite(minimumAHImprovementGm3) && minimumAHImprovementGm3 > 0;
    }
};
}
