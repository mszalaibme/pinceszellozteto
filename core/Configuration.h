#pragma once
#include "ClimateData.h"
#include <cmath>
namespace ventilation {
struct Configuration {
    double targetBasementRH = 65;
    double emergencyRH = 80;
    double minHumidityAdvantage = 1.0;
    double stopHumidityAdvantage = 0.4;
    double minBasementTemperature = 10;
    double rhHysteresis = 3;
    Timestamp minimumOnTime = 600000;
    Timestamp minimumOffTime = 300000;
    Timestamp sensorTimeout = 120000;
    bool isValid() const {
        return std::isfinite(targetBasementRH) && std::isfinite(emergencyRH)
            && std::isfinite(minHumidityAdvantage) && std::isfinite(stopHumidityAdvantage)
            && std::isfinite(minBasementTemperature) && std::isfinite(rhHysteresis)
            && targetBasementRH > 0 && targetBasementRH < emergencyRH && emergencyRH <= 100
            && rhHysteresis > 0 && rhHysteresis < targetBasementRH
            && stopHumidityAdvantage >= 0 && minHumidityAdvantage > stopHumidityAdvantage
            && minHumidityAdvantage <= 50
            && minBasementTemperature >= -20 && minBasementTemperature < 50
            && minimumOnTime <= 86400000 && minimumOffTime <= 86400000
            && sensorTimeout > 0 && sensorTimeout <= 3600000;
    }
};
}
