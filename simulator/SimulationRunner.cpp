#include "SimulationRunner.h"
#include "core/Psychrometrics.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
namespace ventilation::simulation {
std::vector<Scenario> scenarios() {
    std::vector<Scenario> s;
    auto add=[&](const char* id,const char* title,const char* description,double bt,double br,double ot,double rh) -> Scenario& {
        Scenario v; v.id=id; v.title=title; v.description=description;
        v.basementT=bt; v.basementRH=br; v.outdoorT=ot; v.outdoorRH=rh;
        s.push_back(v); return s.back();
    };
    add("winter","01 · Téli szárítás","Magas kültéri RH mellett is száríthat a hideg levegő.",14,80,5,90);
    add("summer","02 · Nyári párás idő","Nedves kültér helyett kompresszoros páramentesítés.",16,75,28,65);
    add("dry","03 · Száraz pince","Nincs szárítási igény; alacsony, 0 g/h terhelés.",15,58,20,65).sourceGramPerHour=0;
    add("emergency","04 · Nagyon nedves pince","85% pincei RH és még nedvesebb kültér.",17,85,28,80);
    auto& full=add("tank-full","05 · Megtelt tartály","4.9 L kezdeti szint, 250 g/h nedvességforrás; a konnektor BE maradhat.",17,85,28,75);
    full.initialTankLiters=4.9; full.sourceGramPerHour=250;
    auto& empty=add("empty-tank","06 · Tartály ürítése","Kezdeti tartályhiba, EMPTY_TANK esemény az 1. órában.",17,85,28,75);
    empty.initialTankLiters=4.9; empty.sourceGramPerHour=250; empty.emptyTankAtSeconds=3600;
    add("no-restart","07 · Nincs automatikus restart","Szándékosan téves capability-beállítás: konnektor BE, készülék standby.",17,82,28,75).autoRestart=false;
    auto& rapid=add("compressor-protection","08 · Kompresszorvédelem","Kétpercenként változó kültér; a kompresszor OFF idő nem kerülhető meg.",17,85,28,75);
    rapid.rapidWeather=true; rapid.sourceGramPerHour=100;
    add("sensor-failure","09 · Kültéri szenzorhiba","Az 1. órától nincs érvényes kültéri mérés, mindkét AUTO kimenet KI.",17,82,28,75).sensorFailureAtSeconds=3600;
    auto& weather=add("weather-change","10 · Nappal és éjszaka","24 órás profil: meleg-párás nappal után hűvösebb éjszaka.",17,85,28,75);
    weather.dailyWeather=true; weather.sourceGramPerHour=100;
    add("five-liters","11 · Öt liter napi terhelés","5000 g/nap falnedvesség, téli kültér. A kezelt víz nem feltétlenül a tartályba kerül.",14,80,5,90).sourceGramPerHour=5000.0/24;
    auto& ineffective=add("ineffective","12 · Hatástalan páramentesítő","A kompresszor fogyaszt, de a modell vízkivonása nulla.",17,85,28,75);
    ineffective.performanceFactor=0; ineffective.sourceGramPerHour=100;
    return s;
}
SimulationResult runScenario(const Scenario& s,SimulationOptions options) {
    if(!std::isfinite(options.hours) || options.hours*3600<1 || options.hours>168
        || !std::isfinite(options.stepSeconds) || options.stepSeconds<1 || options.stepSeconds>60
        || !std::isfinite(options.sourceOverride) || (options.sourceOverride<0 && options.sourceOverride!=-1) || options.sourceOverride>100000)
        throw std::invalid_argument("hours: (0,168], step: [1,60] seconds, source: -1 or [0,100000] g/h");
    Configuration cfg; cfg.dehumidifierAutoRestartSupported=true; cfg.tankEstimationEnabled=true;
    // In no-restart the capability is intentionally misconfigured to test real feedback.
    DryingController controller(cfg);
    const double source=options.sourceOverride>=0?options.sourceOverride:s.sourceGramPerHour;
    BasementModel cellar({26,100,source},s.basementT,s.basementRH);
    DehumidifierParameters dp; dp.initialTankLevelLiters=s.initialTankLiters;
    dp.autoRestartAfterPowerRestore=s.autoRestart; dp.performanceFactor=s.performanceFactor;
    DehumidifierModel dehum(dp);
    SimulationResult result; result.scenario=s;
    if(options.sourceOverride>=0)
        result.scenario.description+=" Egyedi falforrás: "+std::to_string(source)+" g/h.";
    bool emptied=false;
    const Timestamp end=static_cast<Timestamp>(options.hours*3600000);
    const Timestamp increment=static_cast<Timestamp>(options.stepSeconds*1000);
    for(Timestamp now=0;;) {
        const double seconds=static_cast<double>(now)/1000;
        if(!emptied && s.emptyTankAtSeconds>=0 && seconds>=s.emptyTankAtSeconds) {
            dehum.emptyTank(); controller.resetTankEstimate(); emptied=true;
        }
        double ot=s.outdoorT, rh=s.outdoorRH;
        if(s.dailyWeather) { ot=18+10*std::cos(seconds/86400*6.283185307179586); rh=75; }
        if(s.rapidWeather) { const bool wet=(now/120000)%2==0; ot=wet?28:5; rh=wet?75:90; }
        ClimateReading outside{ot,rh,now,true}, sensor=outside;
        if(s.sensorFailureAtSeconds>=0 && seconds>=s.sensorFailureAtSeconds) sensor.valid=false;
        const auto b=cellar.reading(now);
        const auto feedback=dehum.feedback(now);
        const auto output=controller.update(b,sensor,feedback,now);
        result.rows.push_back({now,b,sensor,output,feedback,dehum.tankLevelLiters(),dehum.powerWatts(),dehum.tankFull(),dehum.compressorRunning()});
        if(dehum.tankFull() && result.firstTankFullSeconds<0) result.firstTankFullSeconds=seconds;
        if(now==end) break;
        const Timestamp next=std::min(end,now+increment);
        const double dt=static_cast<double>(next-now)/1000;
        if(output.ventilationRequested) result.fanHours+=dt/3600;
        if(output.dehumidifierRequested) result.dehumCommandHours+=dt/3600;
        // Operator splitting: source/exchange, appliance extraction, then thermal evolution.
        cellar.step(outside,output.ventilationRequested,dt);
        dehum.step(output.dehumidifierRequested,cellar,dt);
        cellar.evolveTemperature(outside,output.ventilationRequested,dehum.powerWatts(),dt,s.basementT);
        now=next;
    }
    const auto& w=cellar.balance();
    result.compressorHours=dehum.runtimeSeconds()/3600;
    result.extractedLiters=dehum.totalExtractedLiters();
    result.generatedLiters=w.generatedGram/1000;
    result.netVentilationLiters=(w.outgoingGram-w.incomingGram)/1000;
    result.condensedLiters=w.condensedGram/1000;
    result.massBalanceErrorGram=cellar.vaporMassGram()+w.outgoingGram+w.condensedGram+w.extractedGram
        -w.initialVaporGram-w.generatedGram-w.incomingGram;
    return result;
}
void writeCsv(const SimulationResult& result,const std::filesystem::path& path) {
    std::ofstream f(path);
    if(!f) throw std::runtime_error("Cannot write CSV: "+path.u8string());
    f << "timestamp,basementTemp,basementRH,basementAH,basementDewPoint,outsideTemp,outsideRH,outsideAH,outsideDewPoint,deltaAH,ventilationCommand,dehumidifierCommand,dehumidifierFeedback,dehumidifierPowerW,tankLevelLiters,tankFull,controllerState,reason,sensorValid,compressorRunning,estimatedTankLiters,ineffectiveWarning\n";
    f << std::setprecision(12);
    auto number=[&](double v) { if(std::isfinite(v)) f<<v; }; // Empty CSV field for unavailable values.
    for(const auto& r:result.rows) {
        const auto b=psychrometrics::calculate(r.basement.temperatureC,r.basement.humidityRH);
        const auto o=psychrometrics::calculate(r.outside.temperatureC,r.outside.humidityRH);
        f<<static_cast<double>(r.timestamp)/1000<<','<<r.basement.temperatureC<<','<<r.basement.humidityRH<<','<<b->absoluteHumidityGm3<<',';
        if(b->dewPointC) number(*b->dewPointC);
        f<<',';
        if(r.outside.valid) f<<r.outside.temperatureC;
        f<<','; if(r.outside.valid) f<<r.outside.humidityRH;
        f<<','; if(r.outside.valid) f<<o->absoluteHumidityGm3;
        f<<','; if(r.outside.valid && o->dewPointC) number(*o->dewPointC);
        f<<','; number(r.output.differenceGm3);
        f<<','<<r.output.ventilationRequested<<','<<r.output.dehumidifierRequested<<','<<feedbackName(r.feedback.feedback)
            <<','<<r.powerWatts<<','<<r.tankLiters<<','<<r.tankFull<<','<<stateName(r.output.state)<<','<<r.output.reason
            <<','<<r.outside.valid<<','<<r.compressorRunning<<','<<r.output.estimatedTankLevelLiters<<','<<r.output.dehumidifierIneffective<<'\n';
    }
    if(!f) throw std::runtime_error("CSV write failed");
}
}
