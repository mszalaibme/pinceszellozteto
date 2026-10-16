#include "BatchSimulation.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
namespace ventilation::simulation {
std::vector<Experiment> baselineExperiments(const SimulationConfiguration& base,double hours) {
    if(!base.isValid()) throw std::invalid_argument("Invalid baseline configuration");
    std::vector<Experiment> jobs;
    for(double ingress:base.sweeps.ingresses) {
        auto c=base; c.moistureIngressGPerHour=ingress;
        if(!c.isValid()) throw std::invalid_argument("Invalid baseline ingress");
        std::ostringstream identifier; identifier<<std::setprecision(17)<<ingress;
        auto s=baselineScenario(c); s.id="baseline-"+identifier.str()+"g";
        s.title="Téli baseline · "+std::to_string(ingress)+" g/h";
        jobs.push_back({"baseline",s.id,s,c,hours});
    }
    return jobs;
}
std::vector<Experiment> sensitivityExperiments(const SimulationConfiguration& base,double hours) {
    if(!base.isValid()) throw std::invalid_argument("Invalid sensitivity configuration");
    const auto& g=base.sweeps;
    const double factorialCount=static_cast<double>(g.airflows.size())*static_cast<double>(g.extractions.size())
        *static_cast<double>(g.ingresses.size())*static_cast<double>(g.targetRHs.size())*static_cast<double>(g.minimumTemperatures.size());
    if(factorialCount>10000) throw std::invalid_argument("Sweep has more than 10000 combinations");
    std::vector<Experiment> jobs;
    auto add=[&](const std::string& group,const SimulationConfiguration& c,bool wet=false) {
        if(!c.isValid()) throw std::invalid_argument("Invalid parameter in sweep group: "+group);
        auto s=wet?scenarios().at(1):baselineScenario(c);
        s.id=group+"-"+std::to_string(jobs.size()+1); s.title=s.id;
        jobs.push_back({group,s.id,s,c,hours});
    };
    for(double airflow:g.airflows) for(double extraction:g.extractions) for(double ingress:g.ingresses)
    for(double target:g.targetRHs) for(double minimum:g.minimumTemperatures) {
        auto c=base; c.ventilationAirflowM3h=airflow; c.dehumidifier.referenceExtractionLh=extraction;
        c.moistureIngressGPerHour=ingress; c.controller.targetBasementRH=target; c.controller.minBasementTemperature=minimum;
        add("factorial",c);
    }
    auto wetBase=base; wetBase.moistureIngressGPerHour=base.wetSensitivityIngressGPerHour;
    wetBase.tankEmptyIntervalHours=base.wetSensitivityTankEmptyIntervalHours;
    for(double extraction:g.extractions) { auto c=wetBase; c.dehumidifier.referenceExtractionLh=extraction; add("extraction-wet",c,true); }
    for(double power:g.powers) {
        auto c=base; c.dehumidifier.powerWatts=power; add("power-baseline",c);
        c=wetBase; c.dehumidifier.powerWatts=power; add("power-wet",c,true);
    }
    for(double capacity:g.buildingCapacities) for(double wall:g.wallTemperatures) for(double conductance:g.wallConductances) {
        auto c=base; c.thermal.effectiveThermalCapacityJPerK=capacity;
        c.thermal.wallTemperatureC=wall; c.thermal.wallAirHeatTransferWPerK=conductance; add("thermal",c);
    }
    for(bool latent:{false,true}) { auto c=wetBase; c.thermal.includeDehumidifierLatentHeat=latent; add("latent-wet",c,true); }
    auto observation=calibrationObservationScenario();
    auto fit=base; fit.moistureIngressGPerHour=base.observationFitIngressGPerHour;
    observation.sourceGramPerHour=fit.moistureIngressGPerHour;
    observation.description="Empirikus hipotézis, nem mért T/RH: kézi készüléküzem; a konfigurált falforrással próbáljuk a kb. 24 órás telítődést reprodukálni.";
    jobs.push_back({"observation-fit",observation.id,observation,fit,std::min(hours,30.0)});
    return jobs;
}
void writeSummaryHeader(std::ostream& f) {
    f<<"group,runId,hours,stepSeconds,basementVolumeM3,ventilationAirflowM3h,referenceExtractionLh,referenceTemperatureC,referenceRH,moistureIngressGPerHour,targetRH,minBasementTemperature,dehumidifierPowerW,ventilationPowerW,fanEnergyIncluded,effectiveThermalCapacityJPerK,airThermalCapacityJPerK,wallTemperatureC,wallAirHeatTransferWPerK,includeDehumidifierLatentHeat,tankEmptyIntervalHours,averageBasementRH,maximumBasementRH,minimumBasementTemperature,ventilationRuntimeHours,dehumidifierRuntimeHours,dehumidifierCommandHours,waterRemovedByVentilationL,waterRemovedByDehumidifierL,tankFullEvents,firstTankFullHours,estimatedElectricityKWh,dehumidifierEnergyKWh,ventilationEnergyKWh,timeAbove70RHHours,timeAbove80RHHours,timeBelowMinimumTemperatureHours,condensedLiters,generatedLiters,vaporStorageChangeGram,massBalanceErrorGram,thermalBalanceErrorJ,emptiedTankLiters,finalTankLevelLiters,energyIsPartial,powerStatus,referenceExtractionIsRated,standbyPowerW,physicalTimeStepSeconds,controllerUpdatePeriodSeconds\n";
}
void writeSummaryRow(std::ostream& f,const SimulationResult& r,const std::string& group) {
    const auto& c=r.configuration;
    f<<std::setprecision(12)<<group<<','<<r.scenario.id<<','<<r.hours<<','<<r.stepSeconds<<','<<c.basementVolumeM3
        <<','<<c.ventilationAirflowM3h<<','<<c.dehumidifier.referenceExtractionLh<<','<<c.dehumidifier.referenceTemperatureC
        <<','<<c.dehumidifier.referenceRH<<','<<c.moistureIngressGPerHour<<','<<c.controller.targetBasementRH
        <<','<<c.controller.minBasementTemperature<<','<<c.dehumidifier.powerWatts<<','<<c.ventilationPowerW<<','<<(c.ventilationPowerW>0)
        <<','<<c.thermal.effectiveThermalCapacityJPerK<<','<<c.basementVolumeM3*c.thermal.airDensityKgM3*c.thermal.airSpecificHeatJPerKgK
        <<','<<c.thermal.wallTemperatureC<<','<<c.thermal.wallAirHeatTransferWPerK<<','<<c.thermal.includeDehumidifierLatentHeat
        <<','<<c.tankEmptyIntervalHours<<','<<r.averageBasementRH<<','<<r.maximumBasementRH<<','<<r.minimumBasementTemperature
        <<','<<r.fanHours<<','<<r.compressorHours<<','<<r.dehumCommandHours<<','<<r.netVentilationLiters<<','<<r.extractedLiters
        <<','<<r.tankFullEventCount<<',';
    if(r.firstTankFullSeconds>=0) f<<r.firstTankFullSeconds/3600;
    f<<','<<r.cumulativeEnergyKWh<<','<<r.dehumidifierEnergyKWh<<','<<r.ventilationEnergyKWh<<','<<r.timeAbove70RH
        <<','<<r.timeAbove80RH<<','<<r.timeBelowMinimumTemperature<<','<<r.condensedLiters<<','<<r.generatedLiters
        <<','<<r.vaporStorageChangeGram<<','<<r.massBalanceErrorGram<<','<<r.thermalBalanceErrorJ<<','<<r.emptiedTankLiters<<','<<r.finalTankLevelLiters<<','<<(c.ventilationPowerW==0 && r.fanHours>0)<<",SIMULATION_ASSUMPTION,0,"<<c.dehumidifier.standbyPowerWatts<<','<<c.physicalTimeStepSeconds<<','<<c.controllerUpdatePeriodSeconds<<'\n';
}
void runBatch(const SimulationConfiguration& base,const std::filesystem::path& output,double step,double hours,bool baselineOnly) {
    auto jobs=baselineExperiments(base,hours);
    if(!baselineOnly) { const auto extra=sensitivityExperiments(base,hours); jobs.insert(jobs.end(),extra.begin(),extra.end()); }
    if(!std::isfinite(step) || step<1 || step>60 || !std::isfinite(hours) || hours<=0 || hours>168)
        throw std::invalid_argument("Invalid batch duration or step");
    double ticks=0,retainedTicks=0;
    for(const auto& job:jobs) {
        const double count=job.hours*3600/step+1;
        ticks+=job.hours*3600*(1/job.configuration.physicalTimeStepSeconds+1/job.configuration.controllerUpdatePeriodSeconds+1/step)+1;
        if(job.group=="baseline" || job.group=="observation-fit") retainedTicks+=count;
    }
    if(ticks>150000000 || retainedTicks>250000) throw std::invalid_argument("Batch too large: increase step or reduce hours/grid");
    std::filesystem::create_directories(output);
    std::ofstream summary(output/"sensitivity-summary.csv"); if(!summary) throw std::runtime_error("Cannot write batch summary");
    writeSummaryHeader(summary);
    std::vector<SimulationResult> detailed;
    std::vector<std::pair<std::string,SimulationResult>> metrics;
    for(std::size_t i=0;i<jobs.size();++i) {
        const auto& job=jobs[i]; SimulationOptions options;
        options.hours=job.hours; options.stepSeconds=step; options.configuration=job.configuration;
        options.retainRows=job.group=="baseline" || job.group=="observation-fit";
        auto r=runScenario(job.scenario,options);
        if(std::abs(r.massBalanceErrorGram)>1e-5 || std::abs(r.thermalBalanceErrorJ)>0.1)
            throw std::runtime_error("Physical balance failed in experiment "+job.id);
        writeSummaryRow(summary,r,job.group);
        auto compact=r; compact.rows.clear(); compact.rows.shrink_to_fit();
        metrics.emplace_back(job.group,std::move(compact));
        if(options.retainRows) { writeCsv(r,output/(r.scenario.id+".csv")); detailed.push_back(std::move(r)); }
        if((i+1)%100==0 || i+1==jobs.size()) std::cout<<"Batch "<<i+1<<'/'<<jobs.size()<<'\n';
    }
    if(!summary) throw std::runtime_error("Batch summary write failed");
    writeDashboard(detailed,output/"report.html");
    writeSweepReport(metrics,output/"analysis.html");
}
}
