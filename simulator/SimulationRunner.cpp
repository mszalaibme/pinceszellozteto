#include "SimulationRunner.h"
#include "core/Psychrometrics.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <limits>
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
Scenario baselineScenario(const SimulationConfiguration& c) {
    Scenario s; s.id="baseline-7day"; s.title="7 napos téli baseline";
    s.description="26.4 m³ referencia, fokozatos téli napi profil. A falforrás és minden modellparaméter konfigurálható.";
    s.basementT=c.initialBasementTemperatureC; s.basementRH=c.initialBasementRH;
    s.initialTankLiters=c.initialTankLevelLiters; s.sourceGramPerHour=c.moistureIngressGPerHour; s.baselineWeather=true;
    return s;
}
Scenario calibrationObservationScenario() {
    Scenario s; s.id="observation-fit"; s.title="5 L / kb. 24 h · kalibrációs hipotézis";
    s.description="Kizárólag empirikus demonstráció: kézi páramentesítés, 212 g/h falforrás. A 15°C/80% kezdet és a termikus paraméterek nem valós mérési adatok.";
    s.basementT=15; s.basementRH=80; s.outdoorT=5; s.outdoorRH=90;
    s.sourceGramPerHour=212; s.mode=Mode::ManualDehumidifier;
    return s;
}
namespace {
double durationAbove(double a,double b,double threshold,double seconds) {
    if(a>threshold && b>threshold) return seconds;
    if(a<=threshold && b<=threshold) return 0;
    const double fraction=(threshold-a)/(b-a);
    return (b>a?1-fraction:fraction)*seconds;
}
}
SimulationResult runScenario(const Scenario& s,SimulationOptions options) {
    if(!std::isfinite(options.hours) || options.hours*3600<1 || options.hours>168
        || !std::isfinite(options.stepSeconds) || options.stepSeconds<1 || options.stepSeconds>60
        || !std::isfinite(options.sourceOverride) || (options.sourceOverride<0 && options.sourceOverride!=-1) || options.sourceOverride>100000)
        throw std::invalid_argument("hours: (0,168], step: [1,60] seconds, source: -1 or [0,100000] g/h");
    auto c=options.configuration.value_or(SimulationConfiguration{});
    if(!c.isValid()) throw std::invalid_argument("Invalid simulation configuration");
    const double source=options.sourceOverride>=0?options.sourceOverride:options.configuration?c.moistureIngressGPerHour:s.sourceGramPerHour;
    c.moistureIngressGPerHour=source;
    DryingController controller(c.controller); controller.setMode(s.mode);
    BasementModel cellar({c.basementVolumeM3,c.ventilationAirflowM3h,source},s.basementT,s.basementRH);
    auto dp=c.dehumidifier;
    dp.initialTankLevelLiters=s.initialTankLiters;
    if(dp.initialTankLevelLiters>dp.tankCapacityLiters) throw std::invalid_argument("Scenario initial tank exceeds configured capacity");
    dp.autoRestartAfterPowerRestore=dp.autoRestartAfterPowerRestore && s.autoRestart;
    dp.performanceFactor*=s.performanceFactor;
    DehumidifierModel dehum(dp);
    SimulationResult result; result.scenario=s; result.configuration=c; result.hours=options.hours; result.stepSeconds=options.stepSeconds;
    result.maximumBasementRH=s.basementRH; result.minimumBasementTemperature=s.basementT;
    result.scenario.description+=" Falforrás: "+std::to_string(source)+" g/h. Modellparaméterek: SIMULATION ASSUMPTION.";
    bool emptied=false;
    const double emptyPeriod=c.tankEmptyIntervalHours*3600;
    double nextEmpty=emptyPeriod>0?emptyPeriod:std::numeric_limits<double>::infinity();
    const Timestamp end=static_cast<Timestamp>(options.hours*3600000);
    const Timestamp increment=static_cast<Timestamp>(options.stepSeconds*1000);
    const Timestamp physicsIncrement=static_cast<Timestamp>(c.physicalTimeStepSeconds*1000);
    const Timestamp controllerPeriod=static_cast<Timestamp>(c.controllerUpdatePeriodSeconds*1000);
    Timestamp nextController=0,nextExport=0,lastExport=0;
    ControllerOutput output;
    bool previousSensorValid=true;
    double exportedVent=0,exportedDehum=0,exportedDeviceEnergy=0,exportedFanEnergy=0;
    for(Timestamp now=0;;) {
        const double seconds=static_cast<double>(now)/1000;
        if(!emptied && s.emptyTankAtSeconds>=0 && seconds>=s.emptyTankAtSeconds) {
            dehum.emptyTank(); controller.resetTankEstimate(); emptied=true;
        }
        if(seconds>=nextEmpty) { dehum.emptyTank(); controller.resetTankEstimate(); nextEmpty+=emptyPeriod; }
        double ot=s.outdoorT, rh=s.outdoorRH;
        if(s.baselineWeather) {
            const auto& w=c.weather;
            const double cycle=std::cos((seconds/3600-w.peakTemperatureHour)/w.periodHours*6.283185307179586);
            ot=(w.minimumTemperatureC+w.maximumTemperatureC)/2+(w.maximumTemperatureC-w.minimumTemperatureC)/2*cycle;
            rh=(w.nightRH+w.dayRH)/2-(w.nightRH-w.dayRH)/2*cycle;
        } else if(s.dailyWeather) { ot=18+10*std::cos(seconds/86400*6.283185307179586); rh=75; }
        if(s.rapidWeather) { const bool wet=(now/120000)%2==0; ot=wet?28:5; rh=wet?75:90; }
        ClimateReading outside{ot,rh,now,true}, sensor=outside;
        if(s.sensorFailureAtSeconds>=0 && seconds>=s.sensorFailureAtSeconds) sensor.valid=false;
        const auto b=cellar.reading(now);
        const auto feedback=dehum.feedback(now);
        if(now>=nextController || (previousSensorValid && !sensor.valid)) {
            output=controller.update(b,sensor,feedback,now);
            nextController=now+controllerPeriod;
        }
        previousSensorValid=sensor.valid;
        const auto& balance=cellar.balance();
        if(now>=nextExport || now==end) {
            auto display=output;
            display.differenceGm3=sensor.valid?psychrometrics::absoluteHumidityGm3(b.temperatureC,b.humidityRH)
                -psychrometrics::absoluteHumidityGm3(sensor.temperatureC,sensor.humidityRH):std::numeric_limits<double>::quiet_NaN();
            SimulationRow row{now,b,sensor,display,feedback,dehum.tankLevelLiters(),dehum.powerWatts(),dehum.tankFull(),dehum.compressorRunning()};
            row.intervalSeconds=static_cast<double>(now-lastExport)/1000;
            const double net=(balance.outgoingGram-balance.incomingGram)/1000;
            if(row.intervalSeconds>0) {
                row.ventilationWaterRemovalGPerHour=(net-exportedVent)*1000*3600/row.intervalSeconds;
                row.dehumidifierWaterRemovalGPerHour=(dehum.totalExtractedLiters()-exportedDehum)*1000*3600/row.intervalSeconds;
                row.averageDehumidifierPowerW=(result.dehumidifierEnergyKWh-exportedDeviceEnergy)*3600000/row.intervalSeconds;
                row.averageVentilationPowerW=(result.ventilationEnergyKWh-exportedFanEnergy)*3600000/row.intervalSeconds;
            }
            row.cumulativeVentilationWaterRemovedL=net; row.cumulativeDehumidifierWaterRemovedL=dehum.totalExtractedLiters();
            row.cumulativeEnergyKWh=result.cumulativeEnergyKWh;
            if(options.retainRows) result.rows.push_back(row);
            ++result.sampleCount;
            exportedVent=net; exportedDehum=dehum.totalExtractedLiters();
            exportedDeviceEnergy=result.dehumidifierEnergyKWh; exportedFanEnergy=result.ventilationEnergyKWh;
            lastExport=now; nextExport=now+increment;
        }
        if(now==end) break;
        const Timestamp next=std::min({end,now+physicsIncrement,nextController,nextExport});
        const double dt=static_cast<double>(next-now)/1000;
        if(output.ventilationRequested) result.fanHours+=dt/3600;
        if(output.dehumidifierRequested) result.dehumCommandHours+=dt/3600;
        const double previousExtracted=dehum.totalExtractedLiters();
        cellar.step(outside,output.ventilationRequested,dt/2);
        dehum.step(output.dehumidifierRequested,cellar,dt);
        cellar.step(outside,output.ventilationRequested,dt/2);
        const double extracted=dehum.totalExtractedLiters()-previousExtracted;
        const double electricalHeat=dehum.intervalEnergyWh()*3600/dt;
        const double latentHeat=c.thermal.includeDehumidifierLatentHeat?extracted*c.thermal.latentHeatJPerKg/dt:0; // 1 L ≈ 1 kg.
        cellar.evolveTemperature(outside,output.ventilationRequested,electricalHeat+latentHeat,dt,c.thermal);
        result.dehumidifierEnergyKWh+=dehum.intervalEnergyWh()/1000;
        if(output.ventilationRequested) result.ventilationEnergyKWh+=c.ventilationPowerW*dt/3600000;
        result.cumulativeEnergyKWh=result.dehumidifierEnergyKWh+result.ventilationEnergyKWh;
        const auto after=cellar.reading(next);
        result.averageBasementRH+=(b.humidityRH+after.humidityRH)/2*dt;
        result.maximumBasementRH=std::max(result.maximumBasementRH,after.humidityRH);
        result.minimumBasementTemperature=std::min(result.minimumBasementTemperature,after.temperatureC);
        result.timeAbove70RH+=durationAbove(b.humidityRH,after.humidityRH,70,dt)/3600;
        result.timeAbove80RH+=durationAbove(b.humidityRH,after.humidityRH,80,dt)/3600;
        result.timeBelowMinimumTemperature+=durationAbove(-b.temperatureC,-after.temperatureC,-c.controller.minBasementTemperature,dt)/3600;
        now=next;
    }
    const auto& w=cellar.balance(); const auto& t=cellar.thermalBalance();
    result.averageBasementRH/=static_cast<double>(end)/1000;
    result.compressorHours=dehum.runtimeSeconds()/3600; result.extractedLiters=dehum.totalExtractedLiters();
    result.generatedLiters=w.generatedGram/1000; result.netVentilationLiters=(w.outgoingGram-w.incomingGram)/1000;
    result.condensedLiters=w.condensedGram/1000;
    result.massBalanceErrorGram=cellar.vaporMassGram()+w.outgoingGram+w.condensedGram+w.extractedGram-w.initialVaporGram-w.generatedGram-w.incomingGram;
    result.thermalBalanceErrorJ=t.sensibleChangeJ+t.wallHeatLossJ+t.ventilationHeatLossJ-t.suppliedHeatJ;
    result.firstTankFullSeconds=dehum.firstTankFullSeconds(); result.tankFullEventCount=dehum.tankFullEvents();
    result.emptiedTankLiters=dehum.emptiedLiters();
    result.vaporStorageChangeGram=cellar.vaporMassGram()-w.initialVaporGram;
    result.finalTankLevelLiters=dehum.tankLevelLiters();
    return result;
}
void writeCsv(const SimulationResult& result,const std::filesystem::path& path) {
    std::ofstream f(path);
    if(!f) throw std::runtime_error("Cannot write CSV: "+path.u8string());
    f << "timestamp,basementTempC,basementRH,basementAH,basementDewPoint,outsideTempC,outsideRH,outsideAH,outsideDewPoint,deltaAH,ventilationCommand,dehumidifierCommand,dehumidifierFeedback,dehumidifierPowerW,tankLevelLiters,tankFull,controllerState,reason,sensorValid,compressorRunning,estimatedTankLiters,ineffectiveWarning,moistureIngressGPerHour,ventilationAirflowM3h,ventilationWaterRemovalGPerHour,dehumidifierWaterRemovalGPerHour,cumulativeVentilationWaterRemovedL,cumulativeDehumidifierWaterRemovedL,cumulativeEnergyKWh,intervalSeconds,averageDehumidifierPowerW,averageVentilationPowerW,energyIsPartial\n";
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
            <<','<<r.outside.valid<<','<<r.compressorRunning<<','<<r.output.estimatedTankLevelLiters<<','<<r.output.dehumidifierIneffective<<','<<result.configuration.moistureIngressGPerHour<<','<<result.configuration.ventilationAirflowM3h
            <<','<<r.ventilationWaterRemovalGPerHour<<','<<r.dehumidifierWaterRemovalGPerHour<<','<<r.cumulativeVentilationWaterRemovedL
            <<','<<r.cumulativeDehumidifierWaterRemovedL<<','<<r.cumulativeEnergyKWh<<','<<r.intervalSeconds<<','<<r.averageDehumidifierPowerW<<','<<r.averageVentilationPowerW
            <<','<<(result.configuration.ventilationPowerW==0 && result.fanHours>0)<<'\n';
    }
    if(!f) throw std::runtime_error("CSV write failed");
}
}
