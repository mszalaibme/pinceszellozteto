#include "SimulationRunner.h"
#include "BatchSimulation.h"
#include "Calibration.h"
#include <fstream>
#include <map>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
using namespace ventilation::simulation;
double numeric(const std::string& s) {
    std::size_t used=0; const double value=std::stod(s,&used);
    if(used!=s.size() || !std::isfinite(value)) throw std::invalid_argument("Invalid numeric argument: "+s);
    return value;
}
int execute(const std::vector<std::string>& args) {
    try {
        SimulationOptions options; std::string selected="all";
        std::filesystem::path output="simulation-output";
        std::optional<std::filesystem::path> configPath,calibrationPath;
        std::map<std::string,double> overrides;
        bool batch=false,baseline=false,hoursExplicit=false;
        for(std::size_t i=1;i<args.size();++i) {
            const auto& a=args[i];
            if(a=="--help" || a=="-h") {
                std::cout<<"Pince simulator 0.4.0 (no hardware required)\n"
                    <<"--baseline : five 7-day ingress cases\n--batch : full factorial + energy + thermal + calibration-point cases\n"
                    <<"--calibrate MEASUREMENTS.ini : inverse water balance / identifiability report\n"
                    <<"--config CONFIG.ini  --scenario all|NAME  --hours HOURS  --step 1..60 (export) --out FOLDER\n"
                    <<"Overrides: --source G_H --airflow M3_H --volume M3 --reference-extraction L_H\n"
                    <<"           --power W --thermal-capacity J_K --wall-temperature C --wall-transfer W_K\n"
                    <<"           --target-rh PERCENT --min-temperature C --empty-every-hours HOURS\n"
                    <<"           --physics-step SECONDS --summary-only (controller cadence is configured separately)\n"
                    <<"--list : scenario names. All uncertain equipment/thermal values are SIMULATION ASSUMPTIONS.\n";
                return 0;
            }
            if(a=="--list") { for(const auto& s:scenarios()) std::cout<<s.id<<" : "<<s.title<<'\n'; std::cout<<"baseline-7day\nobservation-fit\n"; return 0; }
            if(a=="--batch") { batch=true; continue; }
            if(a=="--baseline") { baseline=true; continue; }
            if(a=="--summary-only") { options.retainRows=false; continue; }
            if(i+1>=args.size()) throw std::invalid_argument("Missing value: "+a);
            const auto& value=args[++i];
            if(a=="--scenario") selected=value;
            else if(a=="--hours") { options.hours=numeric(value); hoursExplicit=true; if(options.hours<=0) throw std::invalid_argument("hours must be positive"); }
            else if(a=="--step") options.stepSeconds=numeric(value);
            else if(a=="--source") { options.sourceOverride=numeric(value); if(options.sourceOverride<0) throw std::invalid_argument("source must be nonnegative"); }
            else if(a=="--out") output=std::filesystem::u8path(value);
            else if(a=="--config") configPath=std::filesystem::u8path(value);
            else if(a=="--calibrate") calibrationPath=std::filesystem::u8path(value);
            else if(a=="--airflow" || a=="--volume" || a=="--reference-extraction" || a=="--power" || a=="--thermal-capacity"
                || a=="--wall-temperature" || a=="--wall-transfer" || a=="--target-rh" || a=="--min-temperature" || a=="--empty-every-hours" || a=="--physics-step")
                overrides[a]=numeric(value);
            else throw std::invalid_argument("Unknown argument: "+a);
        }
        auto c=configPath?loadSimulationConfiguration(*configPath):SimulationConfiguration{};
        for(const auto& [key,value]:overrides) {
            if(key=="--airflow") c.ventilationAirflowM3h=value;
            if(key=="--volume") c.basementVolumeM3=value;
            if(key=="--reference-extraction") c.dehumidifier.referenceExtractionLh=value;
            if(key=="--power") c.dehumidifier.powerWatts=value;
            if(key=="--thermal-capacity") c.thermal.effectiveThermalCapacityJPerK=value;
            if(key=="--wall-temperature") c.thermal.wallTemperatureC=value;
            if(key=="--wall-transfer") c.thermal.wallAirHeatTransferWPerK=value;
            if(key=="--target-rh") c.controller.targetBasementRH=value;
            if(key=="--min-temperature") c.controller.minBasementTemperature=value;
            if(key=="--empty-every-hours") c.tankEmptyIntervalHours=value;
            if(key=="--physics-step") c.physicalTimeStepSeconds=value;
        }
        if(!c.isValid()) throw std::invalid_argument("Configuration outside supported limits");
        if((batch && baseline) || (calibrationPath && (batch || baseline))) throw std::invalid_argument("Choose one of --batch, --baseline, --calibrate");
        if(calibrationPath) {
            if(hoursExplicit || options.sourceOverride>=0) throw std::invalid_argument("Calibration duration/water inputs belong in the measurements INI");
            const auto result=calibrate(loadCalibrationMeasurements(*calibrationPath),c);
            writeCalibration(result,output);
            std::cout<<"Average collection="<<result.observedAverageCollectionLh<<" L/h\n";
            for(const auto& warning:result.warnings) std::cout<<"WARNING: "<<warning<<'\n';
            return 0;
        }
        if(options.stepSeconds<1 || options.stepSeconds>60) throw std::invalid_argument("step must be within 1..60 seconds");
        if(batch || baseline) {
            if(options.sourceOverride>=0) throw std::invalid_argument("For a batch configure sweepMoistureIngressGPerHour instead of --source");
            runBatch(c,output,options.stepSeconds,hoursExplicit?options.hours:168,baseline);
            std::cout<<"Ready: "<<std::filesystem::absolute(output/"report.html").u8string()<<'\n'; return 0;
        }
        auto candidates=scenarios();
        if(selected=="baseline-7day") { candidates={baselineScenario(c)}; if(!hoursExplicit) options.hours=168; }
        else if(selected=="observation-fit") {
            candidates={calibrationObservationScenario()}; c.moistureIngressGPerHour=c.observationFitIngressGPerHour;
            options.configuration=c; if(!hoursExplicit) options.hours=30;
        }
        else if(selected!="all") {
            candidates.erase(std::remove_if(candidates.begin(),candidates.end(),[&](const Scenario& s){ return s.id!=selected; }),candidates.end());
            if(candidates.empty()) throw std::invalid_argument("Unknown scenario; use --list");
        }
        if(options.retainRows && options.hours*3600/options.stepSeconds*static_cast<double>(candidates.size())>250000)
            throw std::invalid_argument("Too many samples: increase step or select fewer scenarios (limit 250000 rows)");
        if(configPath || !overrides.empty() || selected=="baseline-7day") options.configuration=c;
        std::filesystem::create_directories(output);
        std::ofstream summary(output/"summary.csv"); if(!summary) throw std::runtime_error("Cannot write summary");
        writeSummaryHeader(summary);
        std::vector<SimulationResult> results;
        for(const auto& s:candidates) {
            results.push_back(runScenario(s,options));
            const auto& r=results.back(); if(options.retainRows) writeCsv(r,output/(s.id+".csv")); writeSummaryRow(summary,r,"scenario");
            std::cout<<s.id<<": mean RH="<<r.averageBasementRH<<"%, min T="<<r.minimumBasementTemperature
                <<" C, fan="<<r.fanHours<<" h, compressor="<<r.compressorHours<<" h, extracted="<<r.extractedLiters<<" L, energy="<<r.cumulativeEnergyKWh<<" kWh\n";
        }
        if(options.retainRows) writeDashboard(results,output/"report.html");
        std::cout<<"Ready: "<<std::filesystem::absolute(output/(options.retainRows?"report.html":"summary.csv")).u8string()<<'\n';
        return 0;
    } catch(const std::exception& e) { std::cerr<<"ERROR: "<<e.what()<<'\n'; return 1; }
}
#ifdef _WIN32
int wmain(int argc,wchar_t** argv) {
    SetConsoleOutputCP(CP_UTF8);
    std::vector<std::string> args;
    for(int i=0;i<argc;++i) {
        const int size=WideCharToMultiByte(CP_UTF8,0,argv[i],-1,nullptr,0,nullptr,nullptr);
        std::string s(static_cast<std::size_t>(size),'\0');
        WideCharToMultiByte(CP_UTF8,0,argv[i],-1,s.data(),size,nullptr,nullptr);
        s.resize(static_cast<std::size_t>(size-1)); args.push_back(s);
    }
    return execute(args);
}
#else
int main(int argc,char** argv) { return execute({argv,argv+argc}); }
#endif
