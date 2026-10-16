#include "Psychrometrics.h"
#include <cmath>
#include <limits>
namespace ventilation::psychrometrics {
namespace {
bool validT(double t) { return std::isfinite(t) && t >= -20 && t <= 50; }
bool valid(double t, double rh) { return validT(t) && std::isfinite(rh) && rh >= 0 && rh <= 100; }
double nan() { return std::numeric_limits<double>::quiet_NaN(); }
}
double saturationVaporPressureHpa(double t) {
    return validT(t) ? 6.112 * std::exp(17.62 * t / (243.12 + t)) : nan();
}
double vaporPressureHpa(double t, double rh) {
    return valid(t, rh) ? saturationVaporPressureHpa(t) * rh / 100.0 : nan();
}
double absoluteHumidityGm3(double t, double rh) {
    return valid(t, rh) ? 216.7 * vaporPressureHpa(t, rh) / (273.15 + t) : nan();
}
std::optional<double> dewPointC(double t, double rh) {
    if (!valid(t, rh) || rh == 0) return std::nullopt;
    const double gamma = std::log(rh / 100.0) + 17.62 * t / (243.12 + t);
    return 243.12 * gamma / (17.62 - gamma);
}
std::optional<ClimateProperties> calculate(double t, double rh) {
    if (!valid(t, rh)) return std::nullopt;
    return ClimateProperties{vaporPressureHpa(t, rh), absoluteHumidityGm3(t, rh), dewPointC(t, rh)};
}
}
