#include "core/VentilationController.h"
#include "Test.h"
#include <limits>
using namespace ventilation;
int main() { return run([] {
    // R1: warmer outdoor air has lower g/m3 but MORE water per mass of dry air.
    VentilationController c;
    c.update({25,100,0,true},{5,90,0,true},0);
    CHECK(c.update({25,100,300000,true},{5,90,300000,true},300000).fanRequested);
    CHECK(!c.update({25,100,300001,true},{38,48,300001,true},300001).fanRequested);
    // R2: user-adjustable thresholds also expose the defect at startup.
    Configuration cfg; cfg.minVentilationHumidityAdvantage=0.1; cfg.stopVentilationHumidityAdvantage=0.01;
    VentilationController startup(cfg);
    startup.update({25,100,0,true},{38,48,0,true},0);
    CHECK(!startup.update({25,100,300000,true},{38,48,300000,true},300000).fanRequested);
    // Every sensor fault must switch a previously running AUTO fan off immediately.
    const ClimateReading goodB{20,90,300001,true}, goodO{5,70,300001,true};
    for (int side=0;side<2;++side) for (int fault=0;fault<12;++fault) {
        VentilationController f;
        f.update({20,90,0,true},{5,70,0,true},0);
        CHECK(f.update({20,90,300000,true},{5,70,300000,true},300000).fanRequested);
        auto b=goodB, o=goodO; auto& bad=side==0 ? b:o;
        switch(fault) {
        case 0: bad.valid=false; break;
        case 1: bad.temperatureC=NAN; break;
        case 2: bad.humidityRH=NAN; break;
        case 3: bad.temperatureC=INFINITY; break;
        case 4: bad.humidityRH=-INFINITY; break;
        case 5: bad.temperatureC=-20.01; break;
        case 6: bad.temperatureC=50.01; break;
        case 7: bad.humidityRH=-0.01; break;
        case 8: bad.humidityRH=100.01; break;
        case 9: bad.measuredAtMs=180001; break; // exact timeout
        case 10: bad.measuredAtMs=300002; break; // future
        case 11: bad.measuredAtMs=0; break; // lost/frozen old sample
        }
        const auto result=f.update(b,o,300001);
        CHECK(!result.fanRequested && result.sensorWarning && result.state==ControllerState::SensorError);
    }
    // Real 64-bit timestamps pass across the 32-bit millis boundary.
    const Timestamp edge=0xFFFFFFFFULL;
    VentilationController timer;
    timer.update({20,90,edge-300000,true},{5,70,edge-300000,true},edge-300000);
    CHECK(timer.update({20,90,edge,true},{5,70,edge,true},edge).fanRequested);
    CHECK(timer.update({20,90,edge+1,true},{5,70,edge+1,true},edge+1).fanRequested);
    // Incorrect raw uint32_t handoff fails OFF rather than interpreting wrap as elapsed time.
    auto wrapped=timer.update({20,90,0,true},{5,70,0,true},0);
    CHECK(!wrapped.fanRequested && wrapped.state==ControllerState::ClockError);
    CHECK(!timer.update({20,90,1,true},{5,70,1,true},1).fanRequested);
    CHECK(timer.update({20,90,300000,true},{5,70,300000,true},300000).fanRequested);
    // UINT64 maximum and wrap: subtraction is guarded, no additive deadline overflow.
    VentilationController huge;
    const auto max=std::numeric_limits<Timestamp>::max();
    huge.update({20,90,max-300000,true},{5,70,max-300000,true},max-300000);
    CHECK(huge.update({20,90,max,true},{5,70,max,true},max).fanRequested);
    CHECK(huge.update({20,90,0,true},{5,70,0,true},0).state==ControllerState::ClockError);
    // Reconstructed core: mode/fan reset; supplied validated config retained.
    cfg.targetBasementRH=70; cfg.ventilationMinimumOffTime=600000;
    VentilationController reboot(cfg);
    CHECK(reboot.mode()==Mode::Auto && reboot.configuration().targetBasementRH==70);
    CHECK(!reboot.update({20,90,0,true},{5,70,0,true},0).fanRequested);
    CHECK(!reboot.update({20,90,599999,true},{5,70,599999,true},599999).fanRequested);
    CHECK(reboot.update({20,90,600000,true},{5,70,600000,true},600000).fanRequested);
    // AH noise crosses both thresholds every second: no restart inside default OFF interval.
    VentilationController chatter;
    bool previous=false; Timestamp offAt=0; int rises=0;
    for (Timestamp now=0;now<=3600000;now+=1000) {
        const bool dry=(now/1000)%2==0;
        const auto out=chatter.update({20,90,now,true},dry?ClimateReading{5,70,now,true}:ClimateReading{38,100,now,true},now);
        if (out.fanRequested && !previous) { CHECK(now-offAt>=300000); ++rises; }
        if (!out.fanRequested && previous) offAt=now;
        previous=out.fanRequested;
    }
    CHECK(rises>0 && rises<=12);
    // Characterization, not a physical guarantee: an air-only cold transient can
    // undershoot the cutoff between samples, even though the next sample stops it.
    VentilationController cooling;
    cooling.update({10.01,100,0,true},{-15,20,0,true},0);
    CHECK(cooling.update({10.01,100,300000,true},{-15,20,300000,true},300000).fanRequested);
    const double afterMinute=-15+(10.01+15)*std::exp(-(100.0/26.0)/60.0);
    CHECK(afterMinute<10); // Explicitly records a remaining sampling/model limitation.
    const auto stopped=cooling.update({afterMinute,100,360000,true},{-15,20,360000,true},360000);
    CHECK(!stopped.fanRequested && stopped.state==ControllerState::BasementTooCold);
    // Cartesian extreme boundaries, including saturated cold and hot outdoor air.
    for (double bt:{5.,10.,10.01,25.}) for(double br:{40.,65.,80.,100.})
    for(double ot:{-15.,0.,25.,38.}) for(double rh:{20.,100.}) {
        VentilationController extreme;
        extreme.update({bt,br,0,true},{ot,rh,0,true},0);
        auto out=extreme.update({bt,br,300000,true},{ot,rh,300000,true},300000);
        CHECK(!out.sensorWarning);
        if(bt<=10 || br<=65) CHECK(!out.fanRequested);
    }
}); }
