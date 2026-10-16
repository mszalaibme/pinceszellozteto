#include "DehumidifierModel.h"
#include "core/Psychrometrics.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace ventilation::simulation {
DehumidifierModel::DehumidifierModel(DehumidifierParameters p) : parameters_(p),tankLiters_(p.initialTankLevelLiters) {
    if(!p.isValid()) throw std::invalid_argument("Invalid empirical dehumidifier parameter");
    if(tankFull()) { fullEvents_=1; firstFullAt_=0; }
}
void DehumidifierModel::stop(double at) {
    if(running_) {
        if(at-runStartedAt_<parameters_.minimumOnTime) ++shortRuns_;
        stoppedAt_=at; running_=false;
    }
}
void DehumidifierModel::emptyTank() {
    emptiedLiters_+=tankLiters_;
    tankLiters_=0;
    commandOnAt_=now_; // A fresh appliance startup delay after the local tank event.
}
void DehumidifierModel::step(bool command,BasementModel& b,double seconds) {
    if(!std::isfinite(seconds) || seconds<=0 || seconds>86400) throw std::invalid_argument("Invalid appliance step");
    if(command!=socket_) {
        if(command) { commandOnAt_=now_; standby_=!parameters_.autoRestartAfterPowerRestore; }
        else { stop(now_); standby_=false; }
        socket_=command;
    }
    intervalEnergyWh_=socket_?parameters_.standbyPowerWatts*seconds/3600:0;
    rate_=parameters_.referenceExtractionLh*parameters_.extractionFactor(b.temperatureC(),b.relativeHumidity())*parameters_.performanceFactor;
    if(!socket_ || standby_ || tankFull()) { stop(now_); now_+=seconds; return; }
    const double ready=running_?now_:std::max(commandOnAt_+parameters_.startupDelay,stoppedAt_+parameters_.minimumOffTime);
    if(ready>=now_+seconds) { now_+=seconds; return; }
    const double runFrom=std::max(now_,ready);
    if(!running_) { running_=true; runStartedAt_=runFrom; }
    double runSeconds=now_+seconds-runFrom;
    // Integrate dM/dt=-A*((RH-RHmin)/(RHref-RHmin))^p exactly at fixed T.
    // RH is derived from M; it is never interpolated or advanced independently.
    const double mass=b.vaporMassGram();
    const double saturation=b.volumeM3()*psychrometrics::absoluteHumidityGm3(b.temperatureC(),100);
    const double floor=saturation*parameters_.minimumExtractionRH/100;
    const double scale=saturation*(parameters_.referenceRH-parameters_.minimumExtractionRH)/100;
    const double z0=std::max(0.0,mass-floor),cap=scale*parameters_.maximumHumidityFactor;
    const double temp=std::clamp((b.temperatureC()-parameters_.minimumExtractionTemperatureC)
        /(parameters_.referenceTemperatureC-parameters_.minimumExtractionTemperatureC),0.0,parameters_.maximumTemperatureFactor);
    const double coefficient=parameters_.referenceExtractionLh*1000*std::pow(temp,parameters_.temperatureExponent)*parameters_.performanceFactor;
    const double exponent=parameters_.humidityExponent;
    auto massAfter=[&](double duration) {
        if(coefficient==0 || z0==0) return mass;
        double z=z0,hours=duration/3600;
        if(z>cap) {
            const double rate=coefficient*std::pow(parameters_.maximumHumidityFactor,exponent);
            const double used=std::min(hours,(z-cap)/rate); z-=rate*used; hours-=used;
        }
        if(hours>0 && z>0) {
            const double k=coefficient/std::pow(scale,exponent);
            if(std::abs(exponent-1)<1e-12) z*=std::exp(-k*hours);
            else {
                const double base=std::pow(z,1-exponent)+(exponent-1)*k*hours;
                z=base>0?std::pow(base,1/(1-exponent)):0;
            }
        }
        return std::clamp(floor+z,0.0,mass);
    };
    const double space=(parameters_.tankCapacityLiters-tankLiters_)*1000;
    double removedGram=mass-massAfter(runSeconds);
    if(removedGram>=space && space>0) {
        double lo=0,hi=runSeconds;
        for(int i=0;i<50;++i) { const double mid=(lo+hi)/2; if(mass-massAfter(mid)>=space) hi=mid; else lo=mid; }
        runSeconds=hi; removedGram=space;
    }
    const double removed=b.removeWaterGram(removedGram)/1000;
    tankLiters_=std::min(parameters_.tankCapacityLiters,tankLiters_+removed);
    totalExtracted_+=removed; runtime_+=runSeconds;
    intervalEnergyWh_+=(parameters_.powerWatts-parameters_.standbyPowerWatts)*runSeconds/3600;
    if(tankFull()) {
        ++fullEvents_;
        if(firstFullAt_<0) firstFullAt_=runFrom+runSeconds;
        stop(runFrom+runSeconds);
    }
    now_+=seconds;
}
DehumidifierReading DehumidifierModel::feedback(Timestamp now) const {
    DehumidifierFeedback f=DehumidifierFeedback::Off;
    if(socket_) {
        if(running_) f=DehumidifierFeedback::Running;
        else if(tankFull() || standby_) f=DehumidifierFeedback::NoPowerDraw;
        else f=DehumidifierFeedback::Starting;
    }
    return {f,powerWatts(),now,true};
}
}
