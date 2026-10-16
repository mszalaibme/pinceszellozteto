#include "SimulationRunner.h"
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
        for(std::size_t i=1;i<args.size();++i) {
            const auto& a=args[i];
            if(a=="--help" || a=="-h") {
                std::cout<<"Pince simulator 0.2.0 (no hardware required)\n"
                    <<"--scenario all|NAME  --hours 24  --step 60  --source G_PER_HOUR  --out FOLDER\n"
                    <<"--list  : show the 12 scenarios\n"
                    <<"Output: report.html and CSV files. Open report.html in your browser.\n";
                return 0;
            }
            if(a=="--list") { for(const auto& s:scenarios()) std::cout<<s.id<<" : "<<s.title<<'\n'; return 0; }
            if(i+1>=args.size()) throw std::invalid_argument("Missing value: "+a);
            const auto& value=args[++i];
            if(a=="--scenario") selected=value;
            else if(a=="--hours") options.hours=numeric(value);
            else if(a=="--step") options.stepSeconds=numeric(value);
            else if(a=="--source") { options.sourceOverride=numeric(value); if(options.sourceOverride<0) throw std::invalid_argument("source must be nonnegative"); }
            else if(a=="--out") output=std::filesystem::u8path(value);
            else throw std::invalid_argument("Unknown argument: "+a);
        }
        auto candidates=scenarios();
        if(selected!="all") {
            candidates.erase(std::remove_if(candidates.begin(),candidates.end(),[&](const Scenario& s){ return s.id!=selected; }),candidates.end());
            if(candidates.empty()) throw std::invalid_argument("Unknown scenario; use --list");
        }
        if(options.stepSeconds<1 || options.stepSeconds>60)
            throw std::invalid_argument("step must be within 1..60 seconds");
        if(options.hours*3600/options.stepSeconds*static_cast<double>(candidates.size())>250000)
            throw std::invalid_argument("Too many samples: use a larger step, fewer hours or a single scenario (limit 250000 rows)");
        std::filesystem::create_directories(output);
        std::vector<SimulationResult> results;
        for(const auto& s:candidates) {
            results.push_back(runScenario(s,options));
            const auto& r=results.back();
            writeCsv(r,output/(s.id+".csv"));
            std::cout<<s.id<<": fan="<<r.fanHours<<" h, socket="<<r.dehumCommandHours
                <<" h, compressor="<<r.compressorHours<<" h, tank extraction="<<r.extractedLiters<<" L\n";
        }
        writeDashboard(results,output/"report.html");
        std::cout<<"Ready: "<<std::filesystem::absolute(output/"report.html").u8string()<<'\n';
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
