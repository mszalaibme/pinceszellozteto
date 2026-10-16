#include "SimulationConfiguration.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
#include <stdexcept>
#include <sstream>
namespace ventilation::simulation {
double DehumidifierParameters::extractionFactor(double t,double rh) const {
    const double tf=std::clamp((t-minimumExtractionTemperatureC)/(referenceTemperatureC-minimumExtractionTemperatureC),0.0,maximumTemperatureFactor);
    const double hf=std::clamp((rh-minimumExtractionRH)/(referenceRH-minimumExtractionRH),0.0,maximumHumidityFactor);
    return std::pow(tf,temperatureExponent)*std::pow(hf,humidityExponent);
}
bool DehumidifierParameters::isValid() const {
    for(double v:{referenceExtractionLh,referenceTemperatureC,referenceRH,minimumExtractionTemperatureC,
        minimumExtractionRH,temperatureExponent,humidityExponent,maximumTemperatureFactor,maximumHumidityFactor,
        tankCapacityLiters,initialTankLevelLiters,powerWatts,standbyPowerWatts,minimumOnTime,minimumOffTime,startupDelay,performanceFactor})
        if(!std::isfinite(v)) return false;
    return referenceExtractionLh>=0 && referenceExtractionLh<=100 && referenceTemperatureC>minimumExtractionTemperatureC
        && minimumExtractionTemperatureC>=-20 && referenceTemperatureC<=50 && referenceRH>minimumExtractionRH
        && minimumExtractionRH>=0 && referenceRH<=100 && temperatureExponent>0 && temperatureExponent<=5
        && humidityExponent>0 && humidityExponent<=5 && maximumTemperatureFactor>=1 && maximumTemperatureFactor<=3
        && maximumHumidityFactor>=1 && maximumHumidityFactor<=3 && tankCapacityLiters>0 && tankCapacityLiters<=100
        && initialTankLevelLiters>=0 && initialTankLevelLiters<=tankCapacityLiters && powerWatts>0 && powerWatts<=100000
        && standbyPowerWatts>=0 && standbyPowerWatts<=powerWatts && minimumOnTime>=0 && minimumOnTime<=86400
        && minimumOffTime>=0 && minimumOffTime<=86400 && startupDelay>=0 && startupDelay<=86400
        && performanceFactor>=0 && performanceFactor<=10;
}
SimulationConfiguration::SimulationConfiguration() {
    controller.dehumidifierAutoRestartSupported=true; // Known simulated model only.
    controller.tankEstimationEnabled=true;
    controller.estimatedExtractionLitersPerHour=0.21;
}
bool SimulationConfiguration::isValid() const {
    for(const auto* values:{&sweeps.airflows,&sweeps.extractions,&sweeps.ingresses,&sweeps.targetRHs,&sweeps.minimumTemperatures,
        &sweeps.powers,&sweeps.buildingCapacities,&sweeps.wallTemperatures,&sweeps.wallConductances}) {
        if(values->empty() || values->size()>20) return false;
        std::set<double> seen;
        for(double v:*values) if(!std::isfinite(v) || !seen.insert(v).second) return false;
    }
    auto range=[](const std::vector<double>& values,double lo,double hi) {
        return std::all_of(values.begin(),values.end(),[&](double v){ return v>=lo && v<=hi; });
    };
    if(!range(sweeps.airflows,0,100000) || !range(sweeps.extractions,0,100) || !range(sweeps.ingresses,0,100000)
        || !range(sweeps.targetRHs,1,controller.emergencyRH-0.01) || !range(sweeps.minimumTemperatures,-20,49.99)
        || !range(sweeps.powers,dehumidifier.standbyPowerWatts,100000) || !range(sweeps.buildingCapacities,0,1e10)
        || !range(sweeps.wallTemperatures,-20,50) || !range(sweeps.wallConductances,0.001,10000)) return false;
    for(double v:{basementVolumeM3,nominalAirflowM3h,ventilationAirflowM3h,moistureIngressGPerHour,initialBasementTemperatureC,
        initialBasementRH,initialTankLevelLiters,ventilationPowerW,tankEmptyIntervalHours,thermal.effectiveThermalCapacityJPerK,
        thermal.wallTemperatureC,thermal.wallAirHeatTransferWPerK,thermal.airDensityKgM3,thermal.airSpecificHeatJPerKgK,
        thermal.latentHeatJPerKg,weather.minimumTemperatureC,weather.maximumTemperatureC,weather.nightRH,weather.dayRH,
        weather.peakTemperatureHour,weather.periodHours,observationFitIngressGPerHour,wetSensitivityIngressGPerHour,
        wetSensitivityTankEmptyIntervalHours,physicalTimeStepSeconds,controllerUpdatePeriodSeconds}) if(!std::isfinite(v)) return false;
    return basementVolumeM3>=0.1 && basementVolumeM3<=100000 && nominalAirflowM3h>=0 && ventilationAirflowM3h>=0
        && ventilationAirflowM3h<=100000 && moistureIngressGPerHour>=0 && moistureIngressGPerHour<=100000
        && initialBasementTemperatureC>=-20 && initialBasementTemperatureC<=50 && initialBasementRH>=0 && initialBasementRH<=100
        && initialTankLevelLiters>=0 && initialTankLevelLiters<=dehumidifier.tankCapacityLiters
        && ventilationPowerW>=0 && ventilationPowerW<=100000 && (tankEmptyIntervalHours==0 || tankEmptyIntervalHours>=1.0/60) && tankEmptyIntervalHours<=168
        && thermal.effectiveThermalCapacityJPerK>=0 && thermal.effectiveThermalCapacityJPerK<=1e10
        && thermal.wallTemperatureC>=-20 && thermal.wallTemperatureC<=50 && thermal.wallAirHeatTransferWPerK>0
        && thermal.wallAirHeatTransferWPerK<=10000 && thermal.airDensityKgM3>0 && thermal.airDensityKgM3<=3
        && thermal.airSpecificHeatJPerKgK>=500 && thermal.airSpecificHeatJPerKgK<=2000
        && thermal.latentHeatJPerKg>=0 && thermal.latentHeatJPerKg<=3000000
        && weather.minimumTemperatureC>=-20 && weather.maximumTemperatureC<=50 && weather.minimumTemperatureC<=weather.maximumTemperatureC
        && weather.nightRH>=0 && weather.nightRH<=100 && weather.dayRH>=0 && weather.dayRH<=100
        && weather.peakTemperatureHour>=0 && weather.peakTemperatureHour<24 && weather.periodHours>0 && weather.periodHours<=168
        && observationFitIngressGPerHour>=0 && observationFitIngressGPerHour<=100000
        && physicalTimeStepSeconds>=1 && physicalTimeStepSeconds<=60 && controllerUpdatePeriodSeconds>=1 && controllerUpdatePeriodSeconds<=60
        && wetSensitivityIngressGPerHour>=0 && wetSensitivityIngressGPerHour<=100000
        && wetSensitivityTankEmptyIntervalHours>=1.0/60 && wetSensitivityTankEmptyIntervalHours<=168
        && dehumidifier.isValid() && controller.isValid();
}
namespace {
std::string trim(std::string s) {
    const auto first=s.find_first_not_of(" \t\r\n"), last=s.find_last_not_of(" \t\r\n");
    return first==std::string::npos?"":s.substr(first,last-first+1);
}
}
double parseNumber(const std::string& s) {
    std::size_t end=0; const double value=std::stod(s,&end);
    if(end!=s.size() || !std::isfinite(value)) throw std::invalid_argument("Invalid finite number: "+s);
    return value;
}
bool parseBoolean(const std::string& s) {
    if(s=="true" || s=="1") return true;
    if(s=="false" || s=="0") return false;
    throw std::invalid_argument("Boolean must be true or false: "+s);
}
IniValues readIni(const std::filesystem::path& path) {
    std::ifstream file(path); if(!file) throw std::runtime_error("Cannot read configuration: "+path.u8string());
    IniValues values; std::string line; unsigned lineNumber=0;
    while(std::getline(file,line)) {
        ++lineNumber;
        if(lineNumber==1 && line.compare(0,3,"\xEF\xBB\xBF")==0) line.erase(0,3);
        const auto comment=line.find_first_of("#;"); if(comment!=std::string::npos) line.erase(comment);
        line=trim(line); if(line.empty()) continue;
        const auto eq=line.find('=');
        if(eq==std::string::npos) throw std::invalid_argument("Expected key=value at line "+std::to_string(lineNumber));
        const auto key=trim(line.substr(0,eq)), value=trim(line.substr(eq+1));
        if(key.empty() || value.empty() || !values.emplace(key,value).second)
            throw std::invalid_argument("Empty or duplicate configuration key at line "+std::to_string(lineNumber));
    }
    return values;
}
SimulationConfiguration loadSimulationConfiguration(const std::filesystem::path& path) {
    SimulationConfiguration c;
    for(const auto& [k,v]:readIni(path)) {
        auto n=[&]{return parseNumber(v);};
        auto ms=[&]{ const double seconds=n(); if(seconds<0 || seconds>604800) throw std::invalid_argument("Invalid timer: "+k); return static_cast<Timestamp>(seconds*1000); };
        auto list=[&] { std::vector<double> values; std::istringstream in(v); std::string part;
            while(std::getline(in,part,',')) values.push_back(parseNumber(trim(part)));
            if(v.empty() || v.back()==',') throw std::invalid_argument("Empty sweep value: "+k); return values; };
        if(k=="basementVolumeM3") c.basementVolumeM3=n();
        else if(k=="nominalAirflowM3h") c.nominalAirflowM3h=n();
        else if(k=="ventilationAirflowM3h") c.ventilationAirflowM3h=n();
        else if(k=="moistureIngressGPerHour") c.moistureIngressGPerHour=n();
        else if(k=="initialBasementTemperatureC") c.initialBasementTemperatureC=n();
        else if(k=="initialBasementRH") c.initialBasementRH=n();
        else if(k=="initialTankLevelLiters") c.initialTankLevelLiters=n();
        else if(k=="ventilationPowerW") c.ventilationPowerW=n();
        else if(k=="tankEmptyIntervalHours") c.tankEmptyIntervalHours=n();
        else if(k=="physicalTimeStepSeconds") c.physicalTimeStepSeconds=n();
        else if(k=="controllerUpdatePeriodSeconds") c.controllerUpdatePeriodSeconds=n();
        else if(k=="observationFitIngressGPerHour") c.observationFitIngressGPerHour=n();
        else if(k=="wetSensitivityIngressGPerHour") c.wetSensitivityIngressGPerHour=n();
        else if(k=="wetSensitivityTankEmptyIntervalHours") c.wetSensitivityTankEmptyIntervalHours=n();
        else if(k=="sweepAirflowsM3h") c.sweeps.airflows=list();
        else if(k=="sweepReferenceExtractionsLh") c.sweeps.extractions=list();
        else if(k=="sweepMoistureIngressGPerHour") c.sweeps.ingresses=list();
        else if(k=="sweepTargetRH") c.sweeps.targetRHs=list();
        else if(k=="sweepMinimumTemperaturesC") c.sweeps.minimumTemperatures=list();
        else if(k=="energyPowerValuesW") c.sweeps.powers=list();
        else if(k=="thermalCapacityValuesJPerK") c.sweeps.buildingCapacities=list();
        else if(k=="thermalWallTemperaturesC") c.sweeps.wallTemperatures=list();
        else if(k=="thermalConductancesWPerK") c.sweeps.wallConductances=list();
        else if(k=="referenceExtractionLh") c.dehumidifier.referenceExtractionLh=n();
        else if(k=="referenceTemperatureC") c.dehumidifier.referenceTemperatureC=n();
        else if(k=="referenceRH") c.dehumidifier.referenceRH=n();
        else if(k=="performanceFactor") c.dehumidifier.performanceFactor=n();
        else if(k=="minimumExtractionTemperatureC") c.dehumidifier.minimumExtractionTemperatureC=n();
        else if(k=="minimumExtractionRH") c.dehumidifier.minimumExtractionRH=n();
        else if(k=="temperatureExponent") c.dehumidifier.temperatureExponent=n();
        else if(k=="humidityExponent") c.dehumidifier.humidityExponent=n();
        else if(k=="maximumTemperatureFactor") c.dehumidifier.maximumTemperatureFactor=n();
        else if(k=="maximumHumidityFactor") c.dehumidifier.maximumHumidityFactor=n();
        else if(k=="dehumidifierPowerW") c.dehumidifier.powerWatts=n();
        else if(k=="standbyPowerW") c.dehumidifier.standbyPowerWatts=n();
        else if(k=="applianceMinimumOnSeconds") c.dehumidifier.minimumOnTime=n();
        else if(k=="applianceMinimumOffSeconds") c.dehumidifier.minimumOffTime=n();
        else if(k=="applianceStartupDelaySeconds") c.dehumidifier.startupDelay=n();
        else if(k=="autoRestartAfterPowerRestore") c.dehumidifier.autoRestartAfterPowerRestore=parseBoolean(v);
        else if(k=="effectiveThermalCapacityJPerK") c.thermal.effectiveThermalCapacityJPerK=n();
        else if(k=="wallTemperatureC") c.thermal.wallTemperatureC=n();
        else if(k=="wallAirHeatTransferWPerK") c.thermal.wallAirHeatTransferWPerK=n();
        else if(k=="airDensityKgM3") c.thermal.airDensityKgM3=n();
        else if(k=="airSpecificHeatJPerKgK") c.thermal.airSpecificHeatJPerKgK=n();
        else if(k=="includeDehumidifierLatentHeat") c.thermal.includeDehumidifierLatentHeat=parseBoolean(v);
        else if(k=="latentHeatJPerKg") c.thermal.latentHeatJPerKg=n();
        else if(k=="weatherMinimumTemperatureC") c.weather.minimumTemperatureC=n();
        else if(k=="weatherMaximumTemperatureC") c.weather.maximumTemperatureC=n();
        else if(k=="weatherNightRH") c.weather.nightRH=n();
        else if(k=="weatherDayRH") c.weather.dayRH=n();
        else if(k=="weatherPeakTemperatureHour") c.weather.peakTemperatureHour=n();
        else if(k=="weatherPeriodHours") c.weather.periodHours=n();
        else if(k=="targetRH") c.controller.targetBasementRH=n();
        else if(k=="startRHHysteresis") c.controller.rhStartHysteresis=n();
        else if(k=="ventilationStartDeltaAH") c.controller.minVentilationHumidityAdvantage=n();
        else if(k=="ventilationStopDeltaAH") c.controller.stopVentilationHumidityAdvantage=n();
        else if(k=="minBasementTemperature") c.controller.minBasementTemperature=n();
        else if(k=="emergencyRH") c.controller.emergencyRH=n();
        else if(k=="ventilationMinimumOnSeconds") c.controller.ventilationMinimumOnTime=ms();
        else if(k=="ventilationMinimumOffSeconds") c.controller.ventilationMinimumOffTime=ms();
        else if(k=="dehumidifierMinimumOnSeconds") c.controller.dehumidifierMinimumOnTime=ms();
        else if(k=="dehumidifierMinimumOffSeconds") c.controller.dehumidifierMinimumOffTime=ms();
        else if(k=="startupGracePeriodSeconds") c.controller.dehumidifierStartupGracePeriod=ms();
        else if(k=="sensorTimeoutSeconds") c.controller.sensorTimeout=ms();
        else if(k=="monitorTimeoutSeconds") c.controller.dehumidifierMonitorTimeout=ms();
        else if(k=="dehumidifierAutoRestartSupported") c.controller.dehumidifierAutoRestartSupported=parseBoolean(v);
        else if(k=="minimumPowerWatts") c.controller.dehumidifierMinimumPowerWatts=n();
        else if(k=="tankCapacityLiters") c.controller.tankCapacityLiters=c.dehumidifier.tankCapacityLiters=n();
        else if(k=="tankEstimationEnabled") c.controller.tankEstimationEnabled=parseBoolean(v);
        else if(k=="estimatedExtractionLh") c.controller.estimatedExtractionLitersPerHour=n();
        else if(k=="performanceMonitorEnabled") c.controller.performanceMonitorEnabled=parseBoolean(v);
        else if(k=="ineffectiveTimeoutSeconds") c.controller.dehumidifierIneffectiveTimeout=ms();
        else if(k=="minimumAHImprovementGm3") c.controller.minimumAHImprovementGm3=n();
        else throw std::invalid_argument("Unknown simulation configuration key: "+k);
    }
    if(!c.isValid()) throw std::invalid_argument("Simulation configuration is outside supported limits");
    return c;
}
}
