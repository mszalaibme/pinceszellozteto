#include "core/DryingController.h"
#include "core/Psychrometrics.h"
#include "Test.h"
#include <random>
using namespace ventilation;
ClimateReading cellar(Timestamp t,double rh=82,double temp=17) { return {temp,rh,t,true}; }
ClimateReading wet(Timestamp t) { return {27,65,t,true}; }
ClimateReading dry(Timestamp t) { return {5,90,t,true}; }
DehumidifierReading fb(Timestamp t,DehumidifierFeedback f=DehumidifierFeedback::Running,double w=310) { return {f,w,t,true}; }
int main() { return run([] {
    Configuration cfg; cfg.dehumidifierAutoRestartSupported=true;
    CHECK(cfg.isValid());
    DryingController winter(cfg);
    CHECK(!winter.update(cellar(0),dry(0),{},0).ventilationRequested);
    auto out=winter.update(cellar(300000),dry(300000),{},300000);
    CHECK(out.ventilationRequested && !out.dehumidifierRequested);
    CHECK(winter.update(cellar(300001,65),dry(300001),{},300001).ventilationRequested);
    CHECK(!winter.update(cellar(900000,65),dry(900000),{},900000).ventilationRequested);
    CHECK(!winter.update(cellar(900001,67),dry(900001),{},900001).ventilationRequested);
    DryingController summer(cfg);
    summer.update(cellar(0),wet(0),fb(0,DehumidifierFeedback::Off,0),0);
    CHECK(!summer.update(cellar(599999),wet(599999),fb(599999),599999).dehumidifierRequested);
    out=summer.update(cellar(600000),wet(600000),fb(600000,DehumidifierFeedback::Off,0),600000);
    CHECK(out.dehumidifierRequested && !out.ventilationRequested);
    out=summer.update(cellar(720000),wet(720000),fb(720000,DehumidifierFeedback::NoPowerDraw,2),720000);
    CHECK(out.dehumidifierRequested && out.state==ControllerState::DehumidifierError && out.reasonCode==ReasonCode::NoPowerDraw);
    out=summer.update(cellar(720001),wet(720001),fb(720001),720001);
    CHECK(out.state==ControllerState::Dehumidifying); // feedback recovery clears diagnostic
    out=summer.update(cellar(720002,64),wet(720002),fb(720002),720002);
    CHECK(out.dehumidifierRequested); // minimum ON
    CHECK(!summer.update(cellar(1500000,64),wet(1500000),fb(1500000),1500000).dehumidifierRequested);
    CHECK(!summer.update(cellar(1500001),wet(1500001),fb(1500001),1500001).dehumidifierRequested);
    CHECK(summer.update(cellar(2100000),wet(2100000),fb(2100000),2100000).dehumidifierRequested);
    auto broken=cellar(2100001); broken.temperatureC=NAN;
    out=summer.update(broken,wet(2100001),fb(2100001),2100001);
    CHECK(out.state==ControllerState::SensorError && !out.dehumidifierRequested && !out.ventilationRequested);
    summer.setMode(Mode::ManualDehumidifier);
    CHECK(!summer.update(broken,wet(2100002),{},2100002).dehumidifierRequested); // manual also respects compressor OFF
    CHECK(summer.update(broken,wet(2700001),{},2700001).dehumidifierRequested);
    summer.setMode(Mode::ManualVentilation);
    out=summer.update(broken,wet(2700002),{},2700002);
    CHECK(out.ventilationRequested && !out.dehumidifierRequested && out.sensorWarning);
    summer.setMode(Mode::ManualOff);
    out=summer.update(broken,wet(2700003),{},2700003); CHECK(!out.ventilationRequested && !out.dehumidifierRequested);
    DryingController unverified;
    unverified.update(cellar(0),wet(0),{},0);
    out=unverified.update(cellar(600000),wet(600000),{},600000);
    CHECK(!out.dehumidifierRequested && out.reasonCode==ReasonCode::AutoRestartUnverified);
    DryingController reboot(cfg);
    CHECK(reboot.mode()==Mode::Auto);
    CHECK(!reboot.update(cellar(0),wet(0),{},0).dehumidifierRequested);
    const Timestamp wrap=0xFFFFFFFFULL;
    DryingController clock(cfg);
    clock.update(cellar(wrap-600000),wet(wrap-600000),{},wrap-600000);
    CHECK(clock.update(cellar(wrap),wet(wrap),{},wrap).dehumidifierRequested);
    CHECK(clock.update(cellar(wrap+1),wet(wrap+1),{},wrap+1).dehumidifierRequested);
    out=clock.update(cellar(0),wet(0),{},0);
    CHECK(out.state==ControllerState::ClockError && !out.dehumidifierRequested && !out.ventilationRequested);
    cfg.dehumidifierIneffectiveTimeout=60000; cfg.tankEstimationEnabled=true;
    DryingController ineffective(cfg);
    ineffective.update(cellar(0),wet(0),fb(0),0);
    ineffective.update(cellar(600000),wet(600000),fb(600000),600000);
    ineffective.update(cellar(600001),wet(600001),fb(600001),600001);
    out=ineffective.update(cellar(660001),wet(660001),fb(660001),660001);
    CHECK(out.dehumidifierIneffective && out.dehumidifierRequested && out.estimatedTankLevelLiters>0);
    ineffective.resetTankEstimate();
    out=ineffective.update(cellar(660001),wet(660001),fb(660001),660001);
    NEAR(out.estimatedTankLevelLiters,0,0);
    // New controller: both climate sensors fail safe, including stale/future/NaN/invalid data.
    for(int side=0;side<2;++side) for(int fault=0;fault<6;++fault) {
        DryingController safe(cfg); safe.update(cellar(0),wet(0),{},0);
        CHECK(safe.update(cellar(600000),wet(600000),fb(600000),600000).dehumidifierRequested);
        auto bi=cellar(600001), oi=wet(600001); auto& bad=side==0?bi:oi;
        if(fault==0) bad.valid=false;
        if(fault==1) bad.temperatureC=NAN;
        if(fault==2) bad.humidityRH=INFINITY;
        if(fault==3) bad.measuredAtMs=480001;
        if(fault==4) bad.measuredAtMs=600002;
        if(fault==5) bad.humidityRH=101;
        const auto error=safe.update(bi,oi,fb(600001),600001);
        CHECK(error.state==ControllerState::SensorError && !error.ventilationRequested && !error.dehumidifierRequested);
    }
    DryingController monitor(cfg);
    monitor.update(cellar(0),wet(0),{},0);
    out=monitor.update(cellar(600000),wet(600000),{},600000);
    CHECK(out.dehumidifierRequested && out.feedbackUnavailable);
    out=monitor.update(cellar(600001),wet(600001),fb(600001),600001);
    CHECK(!out.feedbackUnavailable);
    out=monitor.update(cellar(720002),wet(720002),fb(600001),720002);
    CHECK(out.feedbackUnavailable && out.dehumidifierRequested); // stale is unknown, not an invented fault
    out=monitor.update(cellar(720003),wet(720003),fb(720003,DehumidifierFeedback::Fault,0),720003);
    CHECK(!out.dehumidifierRequested && out.reasonCode==ReasonCode::DeviceFault);
    // At least 10,000 randomized states for the NEW two-actuator core.
    std::mt19937 rng(20261017);
    auto u=[&](double a,double b) { return a+(b-a)*static_cast<double>(rng())/4294967295.; };
    for(int i=0;i<10000;++i) {
        const double bt=u(5,25),br=u(40,100),ot=u(-15,38),rh=u(20,100);
        DryingController c(cfg);
        c.update({bt,br,0,true},{ot,rh,0,true},{},0);
        out=c.update({bt,br,600000,true},{ot,rh,600000,true},{},600000);
        CHECK(!(out.ventilationRequested && out.dehumidifierRequested));
        if(out.ventilationRequested) {
            CHECK(bt>10 && br>=68);
            CHECK(psychrometrics::vaporPressureHpa(ot,rh)<psychrometrics::vaporPressureHpa(bt,br));
        }
        auto invalid=ClimateReading{bt,br,600001,false};
        auto safe=c.update(invalid,{ot,rh,600001,true},{},600001);
        CHECK(!safe.ventilationRequested && !safe.dehumidifierRequested && safe.state==ControllerState::SensorError);
    }
}); }
