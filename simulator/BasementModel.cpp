#include "BasementModel.h"
#include "core/Psychrometrics.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace ventilation::simulation {
namespace {
bool validT(double t) { return std::isfinite(t) && t>=-20 && t<=50; }
}
BasementModel::BasementModel(ModelParameters p, double t, double rh)
    : parameters_(p), temperatureC_(t), vaporMassGram_(0) {
    if (!std::isfinite(p.volumeM3) || p.volumeM3<0.1 || p.volumeM3>100000
        || !std::isfinite(p.fanAirflowM3PerHour) || p.fanAirflowM3PerHour<0 || p.fanAirflowM3PerHour>100000
        || !std::isfinite(p.moistureGenerationGramPerHour) || p.moistureGenerationGramPerHour<0
        || p.moistureGenerationGramPerHour>100000 || !validT(t)
        || !std::isfinite(rh) || rh<0 || rh>100)
        throw std::invalid_argument("Invalid basement model parameters");
    vaporMassGram_=p.volumeM3*psychrometrics::absoluteHumidityGm3(t,rh);
    balance_.initialVaporGram=vaporMassGram_;
}
double BasementModel::saturationMassGram() const {
    return parameters_.volumeM3*psychrometrics::absoluteHumidityGm3(temperatureC_,100);
}
void BasementModel::setTemperatureC(double t) {
    if (!validT(t)) throw std::invalid_argument("Invalid prescribed temperature");
    temperatureC_=t;
    const double excess=std::max(0.0,vaporMassGram_-saturationMassGram());
    vaporMassGram_-=excess;
    balance_.condensedGram+=excess;
}
double BasementModel::relativeHumidity() const {
    return std::clamp(100*vaporMassGram_/saturationMassGram(),0.0,100.0);
}
ClimateReading BasementModel::reading(Timestamp now) const {
    return {temperatureC_,relativeHumidity(),now,true};
}
double BasementModel::removeWaterGram(double requested) {
    if(!std::isfinite(requested) || requested<0) throw std::invalid_argument("Invalid water extraction");
    const double removed=std::min(requested,vaporMassGram_);
    vaporMassGram_-=removed; balance_.extractedGram+=removed;
    return removed;
}
void BasementModel::evolveTemperature(const ClimateReading& outside,bool fanOn,double heat,double seconds,
    const ThermalParameters& p) {
    const double capacity=p.effectiveThermalCapacityJPerK+parameters_.volumeM3*p.airDensityKgM3*p.airSpecificHeatJPerKgK;
    const double wallT=p.wallTemperatureC, conductance=p.wallAirHeatTransferWPerK;
    if(!outside.isPlausible() || !std::isfinite(heat) || heat<0 || !std::isfinite(seconds) || seconds<0 || seconds>86400
        || !validT(wallT) || !std::isfinite(capacity) || capacity<=0 || !std::isfinite(conductance) || conductance<=0
        || !std::isfinite(p.effectiveThermalCapacityJPerK) || p.effectiveThermalCapacityJPerK<0
        || !std::isfinite(p.airDensityKgM3) || p.airDensityKgM3<=0 || !std::isfinite(p.airSpecificHeatJPerKgK) || p.airSpecificHeatJPerKgK<=0)
        throw std::invalid_argument("Invalid thermal boundary");
    if(seconds==0) return;
    const double airLoss=fanOn ? parameters_.fanAirflowM3PerHour/3600*p.airDensityKgM3*p.airSpecificHeatJPerKgK : 0;
    const double loss=conductance+airLoss;
    const double equilibrium=(conductance*wallT+airLoss*outside.temperatureC+heat)/loss;
    const double old=temperatureC_, rate=loss/capacity;
    const double fraction=-std::expm1(-rate*seconds);
    const double next=old+(equilibrium-old)*fraction;
    // Reject an unsupported physical state; do not silently discard heat by clamping T.
    if(!validT(next)) throw std::runtime_error("Thermal model left -20..50 C: check configuration and heat capacity");
    const double integralT=equilibrium*seconds+(old-equilibrium)*fraction/rate;
    thermalBalance_.suppliedHeatJ+=heat*seconds;
    thermalBalance_.wallHeatLossJ+=conductance*(integralT-wallT*seconds);
    thermalBalance_.ventilationHeatLossJ+=airLoss*(integralT-outside.temperatureC*seconds);
    thermalBalance_.sensibleChangeJ+=capacity*(next-old);
    setTemperatureC(next);
}
void BasementModel::step(const ClimateReading& outside, bool fanOn, double seconds) {
    if (!std::isfinite(seconds) || seconds<0 || seconds>86400)
        throw std::invalid_argument("Step must be finite and within 0..86400 seconds");
    const double q=fanOn ? parameters_.fanAirflowM3PerHour : 0;
    // No outdoor data is needed with zero exchange. Failure injection belongs in
    // the sensor adapter, not in the physical outdoor boundary condition.
    if (q>0 && !outside.isPlausible()) throw std::invalid_argument("Invalid outdoor boundary condition");
    if (seconds==0) return;
    const double hours=seconds/3600;
    const double source=parameters_.moistureGenerationGramPerHour;
    const double cap=saturationMassGram();
    balance_.generatedGram+=source*hours;
    if (q==0) {
        const double next=vaporMassGram_+source*hours;
        balance_.condensedGram+=std::max(0.0,next-cap);
        vaporMassGram_=std::min(next,cap);
        return;
    }
    // At equal total pressure, cooling/heating outdoor air preserves its vapor
    // pressure before condensation. Normalize incoming density to basement T.
    const double incomingDensity=216.7*psychrometrics::vaporPressureHpa(outside.temperatureC,outside.humidityRH)
        /(273.15+temperatureC_);
    const double incomingRate=q*incomingDensity;
    const double k=q/parameters_.volumeM3;
    const double equilibrium=(incomingRate+source)/k;
    const double initial=vaporMassGram_;
    double freeHours=hours;
    if (equilibrium>cap) {
        const double crossing=initial>=cap ? 0 : std::log1p((cap-initial)/(equilibrium-cap))/k;
        freeHours=std::min(hours,crossing);
    }
    // Exact well-mixed solution; expm1 avoids loss of small time increments.
    double next=initial+(equilibrium-initial)*(-std::expm1(-k*freeHours));
    double outgoing=(incomingRate+source)*freeHours-(next-initial);
    const double saturatedHours=hours-freeHours;
    if (saturatedHours>0) {
        next=cap;
        outgoing+=k*cap*saturatedHours;
        balance_.condensedGram+=(incomingRate+source-k*cap)*saturatedHours;
    }
    balance_.incomingGram+=incomingRate*hours;
    balance_.outgoingGram+=outgoing;
    vaporMassGram_=std::clamp(next,0.0,cap);
}
}
