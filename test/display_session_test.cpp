#include "display/DisplaySession.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
// Do not use assert: Release builds define NDEBUG.
void require(bool value) { if(!value) throw std::runtime_error("Display session regression"); }
int main() {
    using namespace ventilation;
    using namespace ventilation::display;
    DisplaySession s;
    require(!s.output().fanRequested && !s.output().dehumidifierRequested);
    require(s.output().reasonCode==ReasonCode::MinimumOff);
    s.advance(300); require(s.output().fanRequested && !s.output().dehumidifierRequested);
    s.setFault(Fault::NaN); require(s.output().sensorWarning && !s.output().fanRequested);
    s.setMode(Mode::ManualVentilation); require(s.output().fanRequested && s.output().sensorWarning);
    s.setMode(Mode::Off); require(!s.output().fanRequested);
    s.preset(1); s.advance(600); require(s.output().dehumidifierRequested && !s.output().fanRequested);
    s.advance(60); require(s.feedback().feedback==DehumidifierFeedback::Running);
    s.setFault(Fault::NoPower); s.advance(121);
    require(s.output().reasonCode==ReasonCode::NoPowerDraw && s.output().dehumidifierRequested);
    s.reboot(); require(s.now()==0 && s.mode()==Mode::Auto && !s.output().dehumidifierRequested);
    s.preset(5); require(s.output().reasonCode==ReasonCode::NoPowerDraw && s.actualTank()==5);
    s.emptyTank(); require(s.actualTank()==0 && s.fault()==Fault::None && s.output().estimatedTankLevelLiters==0);
    s.preset(6); require(s.output().reasonCode==ReasonCode::AutoRestartUnverified && !s.output().dehumidifierRequested);
    s.preset(7); require(s.output().feedbackUnavailable && s.output().dehumidifierRequested);
    auto c=s.configuration(); c.targetBasementRH=95; require(!s.configure(c));
    s.preset(3); require(s.output().state==ControllerState::Idle);
    s.preset(2); s.advance(600); require(!s.output().fanRequested && s.output().dehumidifierRequested);
    s.preset(0); const double rh=s.basement().humidityRH;
    s.advance(600); require(std::abs(s.basement().humidityRH-rh)<1e-9);
    s.setDynamic(true); s.advance(60); require(std::abs(s.basement().humidityRH-rh)>0.01);
    s.setFault(Fault::OutdoorSensor); s.advance(1); require(s.output().sensorWarning && !s.output().fanRequested && !s.output().dehumidifierRequested);
    bool bad=false; try { s.setClimate(15,101,5,90); } catch(const std::invalid_argument&) { bad=true; } require(bad);
    std::cout<<"Display session: real core timers, all presets, manual modes, NaN, feedback, reboot, tank reset, config and mass dynamics passed\n";
}
