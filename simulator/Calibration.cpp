#include "Calibration.h"
#include "core/Psychrometrics.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
namespace ventilation::simulation {
CalibrationMeasurements loadCalibrationMeasurements(const std::filesystem::path& path) {
    CalibrationMeasurements m;
    for(const auto& [k,v]:readIni(path)) {
        if(k=="assumeSteadyState") m.assumeSteadyState=parseBoolean(v);
        else if(k=="assumeNoCondensation") m.assumeNoCondensation=parseBoolean(v);
        else {
            const double n=parseNumber(v);
            if(k=="measuredWaterCollectedLiters") m.measuredWaterCollectedLiters=n;
            else if(k=="measurementDurationHours") m.measurementDurationHours=n;
            else if(k=="measuredBasementTemperature") m.measuredBasementTemperature=n;
            else if(k=="measuredBasementRH") m.measuredBasementRH=n;
            else if(k=="measuredOutdoorTemperature") m.measuredOutdoorTemperature=n;
            else if(k=="measuredOutdoorRH") m.measuredOutdoorRH=n;
            else if(k=="fanRuntimeHours") m.fanRuntimeHours=n;
            else if(k=="dehumidifierRuntimeHours") m.dehumidifierRuntimeHours=n;
            else if(k=="initialBasementTemperature") m.initialBasementTemperature=n;
            else if(k=="initialBasementRH") m.initialBasementRH=n;
            else if(k=="finalBasementTemperature") m.finalBasementTemperature=n;
            else if(k=="finalBasementRH") m.finalBasementRH=n;
            else if(k=="measuredCondensateLiters") m.measuredCondensateLiters=n;
            else throw std::invalid_argument("Unknown calibration key: "+k);
        }
    }
    return m;
}
CalibrationResult calibrate(const CalibrationMeasurements& m,const SimulationConfiguration& c) {
    if(!c.isValid() || !std::isfinite(m.measuredWaterCollectedLiters) || m.measuredWaterCollectedLiters<0
        || !std::isfinite(m.measurementDurationHours) || m.measurementDurationHours<=0 || m.measurementDurationHours>87600)
        throw std::invalid_argument("Calibration requires nonnegative collected water and positive measurement duration");
    for(const auto& runtime:{m.fanRuntimeHours,m.dehumidifierRuntimeHours})
        if(runtime && (!std::isfinite(*runtime) || *runtime<0 || *runtime>m.measurementDurationHours))
            throw std::invalid_argument("Runtime must be in 0..measurement duration");
    if(m.dehumidifierRuntimeHours && *m.dehumidifierRuntimeHours==0 && m.measuredWaterCollectedLiters>0)
        throw std::invalid_argument("Positive extracted water with zero compressor runtime is inconsistent");
    auto climate=[&](const std::optional<double>& t,const std::optional<double>& rh) {
        if(t.has_value()!=rh.has_value()) throw std::invalid_argument("Each climate measurement requires both T and RH");
        if(t && !psychrometrics::calculate(*t,*rh)) throw std::invalid_argument("Invalid calibration climate measurement");
    };
    climate(m.measuredBasementTemperature,m.measuredBasementRH);
    climate(m.measuredOutdoorTemperature,m.measuredOutdoorRH);
    climate(m.initialBasementTemperature,m.initialBasementRH);
    climate(m.finalBasementTemperature,m.finalBasementRH);
    if(m.initialBasementTemperature.has_value()!=m.finalBasementTemperature.has_value())
        throw std::invalid_argument("Provide both initial and final climate, or neither");
    if(m.measuredCondensateLiters && (!std::isfinite(*m.measuredCondensateLiters) || *m.measuredCondensateLiters<0))
        throw std::invalid_argument("Invalid condensate measurement");
    if(m.assumeNoCondensation && m.measuredCondensateLiters && *m.measuredCondensateLiters>0)
        throw std::invalid_argument("Measured condensation contradicts assumeNoCondensation");
    CalibrationResult r;
    if(m.fanRuntimeHours && m.dehumidifierRuntimeHours && *m.fanRuntimeHours+ *m.dehumidifierRuntimeHours>m.measurementDurationHours)
        r.warnings.push_back("Measured runtimes imply overlap; the simulated AUTO controller does not command BOTH.");
    r.observedAverageCollectionLh=m.measuredWaterCollectedLiters/m.measurementDurationHours;
    r.effectiveExtractionLowerBoundLh=r.observedAverageCollectionLh;
    if(m.dehumidifierRuntimeHours && *m.dehumidifierRuntimeHours>0) {
        r.effectiveDehumidifierExtractionLh=m.measuredWaterCollectedLiters/ *m.dehumidifierRuntimeHours;
        if(m.measuredBasementTemperature) {
            const double factor=c.dehumidifier.extractionFactor(*m.measuredBasementTemperature,*m.measuredBasementRH)*c.dehumidifier.performanceFactor;
            if(factor>0) r.fittedReferenceExtractionLh=*r.effectiveDehumidifierExtractionLh/factor;
            else r.warnings.push_back("Reference extraction is not identifiable where the assumed performance factor is zero.");
            r.assumptions.push_back("Reported indoor T/RH represents compressor operating conditions; nonlinear averaging may bias this fit.");
        }
    } else r.warnings.push_back("Actual compressor runtime unknown or zero: collection/duration is only a lower bound on running extraction, not a factory rating.");
    std::optional<double> deltaWater;
    if(m.initialBasementTemperature) {
        deltaWater=c.basementVolumeM3*(psychrometrics::absoluteHumidityGm3(*m.finalBasementTemperature,*m.finalBasementRH)
            -psychrometrics::absoluteHumidityGm3(*m.initialBasementTemperature,*m.initialBasementRH));
    } else if(m.assumeSteadyState) {
        deltaWater=0;
        r.assumptions.push_back("QUASI_STEADY_ASSUMPTION: no change in stored airborne vapor mass.");
    }
    std::optional<double> ventilationWater;
    if(m.fanRuntimeHours) {
        if(*m.fanRuntimeHours==0) ventilationWater=0;
        else if(m.measuredBasementTemperature && m.measuredOutdoorTemperature) {
            const double incoming=216.7*psychrometrics::vaporPressureHpa(*m.measuredOutdoorTemperature,*m.measuredOutdoorRH)/(273.15+ *m.measuredBasementTemperature);
            ventilationWater=c.ventilationAirflowM3h*(psychrometrics::absoluteHumidityGm3(*m.measuredBasementTemperature,*m.measuredBasementRH)-incoming)* *m.fanRuntimeHours;
            r.assumptions.push_back("Mean indoor/outdoor conditions approximate the runtime-weighted ventilation water balance at equal pressure.");
        }
    }
    if(deltaWater && ventilationWater) {
        const double minimum=(*deltaWater+ *ventilationWater+1000*m.measuredWaterCollectedLiters)/m.measurementDurationHours;
        r.moistureIngressLowerBoundGPerHour=minimum;
        if(m.measuredCondensateLiters) r.moistureIngressGPerHour=minimum+1000* *m.measuredCondensateLiters/m.measurementDurationHours;
        else if(m.assumeNoCondensation) {
            r.moistureIngressGPerHour=minimum;
            r.assumptions.push_back("NO_CONDENSATION_ASSUMPTION: no additional unmeasured condensation sink.");
        } else r.warnings.push_back("Unmeasured condensation: moisture ingress is only lower-bounded.");
        if(minimum<0) r.warnings.push_back("Negative unconstrained ingress estimate: check averages, storage change and unmeasured sinks; value was not silently clamped.");
    } else r.warnings.push_back("Moisture ingress not identifiable: fan runtime/representative climate and vapor storage change or explicit steady-state assumption are missing.");
    r.warnings.push_back("One water-collection observation cannot jointly identify airflow, thermal properties, ingress and the full performance curve.");
    return r;
}
void writeCalibration(const CalibrationResult& r,const std::filesystem::path& folder) {
    std::filesystem::create_directories(folder);
    std::ofstream json(folder/"calibration.json"),csv(folder/"calibration.csv");
    if(!json || !csv) throw std::runtime_error("Cannot write calibration results");
    json<<std::setprecision(12)<<"{\n"; csv<<std::setprecision(12)<<"metric,value\n";
    auto metric=[&](const char* key,std::optional<double> value) {
        json<<"  \""<<key<<"\": "; if(value) json<<*value; else json<<"null";
        json<<",\n"; csv<<key<<','; if(value) csv<<*value; csv<<'\n';
    };
    metric("observedAverageCollectionLh",r.observedAverageCollectionLh);
    metric("effectiveExtractionLowerBoundLh",r.effectiveExtractionLowerBoundLh);
    metric("effectiveDehumidifierExtractionLh",r.effectiveDehumidifierExtractionLh);
    metric("fittedReferenceExtractionLh",r.fittedReferenceExtractionLh);
    metric("moistureIngressGPerHour",r.moistureIngressGPerHour);
    metric("moistureIngressLowerBoundGPerHour",r.moistureIngressLowerBoundGPerHour);
    auto messages=[&](const char* key,const std::vector<std::string>& values,bool last) {
        json<<"  \""<<key<<"\": [";
        bool first=true; for(const auto& v:values) { if(!first) json<<','; first=false; json<<'"'<<v<<'"'; }
        json<<']'<<(last?"\n":",\n");
    };
    messages("assumptions",r.assumptions,false); messages("warnings",r.warnings,true); json<<"}\n";
    if(!json || !csv) throw std::runtime_error("Calibration export failed");
}
}
