#pragma once
#include <optional>
namespace ventilation::psychrometrics {
struct ClimateProperties {
    double vaporPressureHpa;
    double absoluteHumidityGm3;
    std::optional<double> dewPointC; // No finite dew point for RH=0.
};
double saturationVaporPressureHpa(double temperatureC);
double vaporPressureHpa(double temperatureC, double humidityRH);
double absoluteHumidityGm3(double temperatureC, double humidityRH);
std::optional<double> dewPointC(double temperatureC, double humidityRH);
std::optional<ClimateProperties> calculate(double temperatureC, double humidityRH);
}
