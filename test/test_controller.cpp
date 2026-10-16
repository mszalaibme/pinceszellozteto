#include "core/VentilationController.h"
#include "core/Psychrometrics.h"
#include "Test.h"
using namespace ventilation;
ClimateReading b(double rh, Timestamp t, double temp = 14) { return {temp, rh, t, true}; }
ClimateReading o(Timestamp t, double temp = 5, double rh = 90) { return {temp, rh, t, true}; }
int main() { return run([] {
    VentilationController c;
    CHECK(c.update(b(80,0),o(0),0).state == ControllerState::MinimumOffTimer);
    CHECK(!c.update(b(80,299999),o(299999),299999).fanRequested);
    CHECK(c.update(b(80,300000),o(300000),300000).fanRequested);
    CHECK(c.update(b(61,300001),o(300001),300001).state == ControllerState::MinimumOnTimer);
    CHECK(c.update(b(61,899999),o(899999),899999).fanRequested);
    CHECK(!c.update(b(61,900000),o(900000),900000).fanRequested);
    CHECK(c.update(b(80,900001),o(900001),900001).state == ControllerState::MinimumOffTimer);
    CHECK(c.update(b(80,1200000),o(1200000),1200000).fanRequested);
    CHECK(!c.update(b(80,1200001),o(1200001,28,65),1200001).fanRequested);
    Configuration cfg; cfg.ventilationMinimumOffTime=0;
    VentilationController cold(cfg); CHECK(cold.update(b(80,0),o(0),0).fanRequested);
    CHECK(cold.update(b(80,1,10),o(1),1).state == ControllerState::BasementTooCold);
    VentilationController stale(cfg); CHECK(stale.update(b(80,0),o(0),0).fanRequested);
    CHECK(stale.update(b(80,120000),o(0),120000).state == ControllerState::SensorError);
    CHECK(!stale.update(b(80,120001),o(0),120001).fanRequested);
    stale.setMode(Mode::On);
    auto manual = stale.update(b(80,120002),o(0),120002);
    CHECK(manual.fanRequested && manual.sensorWarning && manual.state == ControllerState::ManualOn);
    stale.setMode(Mode::Off); CHECK(!stale.update(b(80,120003),o(0),120003).fanRequested);
    stale.setMode(Mode::Auto); CHECK(stale.update(b(80,120004),o(0),120004).state == ControllerState::SensorError);
    VentilationController summer(cfg);
    auto wet = summer.update(b(85,0,16),o(0,28,65),0);
    CHECK(!wet.fanRequested && wet.highHumidityWarning && wet.dehumidificationSuggested);
    CHECK(wet.state == ControllerState::OutdoorAirTooHumid);
    CHECK(!summer.update(b(58,1,15),o(1),1).fanRequested);
    auto bad=b(80,2); bad.temperatureC=NAN;
    CHECK(summer.update(bad,o(2),2).sensorWarning);
    bad=b(101,3); CHECK(summer.update(bad,o(3),3).state == ControllerState::SensorError);
    bad=b(80,5); CHECK(summer.update(bad,o(4),4).state == ControllerState::SensorError);
    CHECK(summer.update(b(80,2),o(2),2).state == ControllerState::ClockError);
    auto invalid=cfg; invalid.sensorTimeout=0;
    CHECK(!summer.setConfiguration(invalid)); CHECK(summer.configuration().sensorTimeout==120000);
    CHECK(summer.setConfiguration(cfg));
    VentilationController invalidConstructor(invalid); CHECK(invalidConstructor.configuration().isValid());
    // Fixed cellar climate, vary outdoor AH to test exact humidity thresholds.
    cfg.ventilationMinimumOnTime=0;
    VentilationController h(cfg);
    const double ah=psychrometrics::absoluteHumidityGm3(14,80);
    auto outsideDelta=[&](double delta,Timestamp t) {
        return ClimateReading{14,100*(ah-delta)/psychrometrics::absoluteHumidityGm3(14,100),t,true};
    };
    CHECK(!h.update(b(80,0),outsideDelta(0.9,0),0).fanRequested);
    CHECK(h.update(b(80,1),outsideDelta(1.01,1),1).fanRequested);
    CHECK(h.update(b(80,2),outsideDelta(0.5,2),2).fanRequested);
    CHECK(!h.update(b(80,3),outsideDelta(0.39,3),3).fanRequested);
    CHECK(!h.update(b(80,4),outsideDelta(0.8,4),4).fanRequested);
    CHECK(h.update(b(80,5),outsideDelta(1.2,5),5).fanRequested);
    CHECK(h.update(b(64,6),o(6),6).fanRequested);
    CHECK(!h.update(b(62,7),o(7),7).fanRequested);
    CHECK(!h.update(b(65,8),o(8),8).fanRequested);
    h.setMode(Mode::On); CHECK(h.update(b(58,9),o(9,28,65),9).fanRequested);
    h.setMode(Mode::Auto); CHECK(!h.update(b(58,10),o(10,28,65),10).fanRequested);
    CHECK(std::string(stateName(ControllerState::Drying))=="DRYING");
}); }
