#include "DisplaySession.h"
#include <limits>
#include <stdexcept>
namespace ventilation::display {
namespace {
Configuration initialConfig() {
    Configuration c;
    // Explicit simulator capabilities; real appliance restart is still unverified.
    c.dehumidifierAutoRestartSupported=true;
    c.tankEstimationEnabled=true;
    return c;
}
}
DisplaySession::DisplaySession() : controller_(initialConfig()),basement_({},14.7,78) { refresh(); sample(); }
bool DisplaySession::configure(const Configuration& c) {
    if(!controller_.setConfiguration(c)) return false;
    refresh(); return true;
}
void DisplaySession::setMode(Mode m) { controller_.setMode(m); refresh(); }
void DisplaySession::setClimate(double bt,double br,double ot,double orh) {
    if(!std::isfinite(bt)||!std::isfinite(br)||!std::isfinite(ot)||!std::isfinite(orh)
        ||bt<5||bt>25||br<40||br>100||ot< -15||ot>38||orh<20||orh>100)
        throw std::invalid_argument("Climate outside simulator range");
    basement_=simulation::BasementModel({},bt,br); outsideT_=ot; outsideRH_=orh;
    refresh(); sample();
}
void DisplaySession::setFault(Fault f) { fault_=f; refresh(); }
ClimateReading DisplaySession::basement() const {
    auto b=basement_.reading(now_);
    if(fault_==Fault::BasementSensor) b.valid=false;
    if(fault_==Fault::NaN) b.temperatureC=std::numeric_limits<double>::quiet_NaN();
    return b;
}
ClimateReading DisplaySession::outside() const { return {outsideT_,outsideRH_,now_,fault_!=Fault::OutdoorSensor}; }
DehumidifierReading DisplaySession::feedback() const {
    auto f=appliance_.feedback(now_);
    if(fault_==Fault::NoPower || fault_==Fault::FullTank) f={DehumidifierFeedback::NoPowerDraw,0,now_,true};
    if(fault_==Fault::Appliance) f={DehumidifierFeedback::Fault,0,now_,true};
    if(fault_==Fault::UnknownFeedback) f={DehumidifierFeedback::Unknown,std::numeric_limits<double>::quiet_NaN(),now_,false};
    return f;
}
void DisplaySession::refresh() { output_=controller_.update(basement(),outside(),feedback(),now_); }
double DisplaySession::actualTank() const { return fault_==Fault::FullTank?5:appliance_.tankLevelLiters(); }
void DisplaySession::sample() {
    history_.push_back({static_cast<double>(now_)/1000,basement_.relativeHumidity(),basement_.temperatureC(),output_.fanRequested,output_.dehumidifierRequested});
    if(history_.size()>1441) history_.pop_front();
}
void DisplaySession::advance(unsigned seconds) {
    if(seconds>86400) throw std::invalid_argument("Advance limited to one day");
    for(unsigned i=0;i<seconds;++i) {
        const bool powered=output_.dehumidifierRequested && fault_!=Fault::NoPower && fault_!=Fault::Appliance && fault_!=Fault::FullTank;
        const auto env=ClimateReading{outsideT_,outsideRH_,now_,true};
        // A held-climate test still runs the appliance clock and tank. Restore the
        // imposed climate after extraction; this is a test boundary, not a mass simulation.
        const double t=basement_.temperatureC(),rh=basement_.relativeHumidity();
        if(dynamic_) basement_.step(env,output_.fanRequested,0.5);
        appliance_.step(powered,basement_,1);
        if(dynamic_) {
            basement_.step(env,output_.fanRequested,0.5);
            basement_.evolveTemperature(env,output_.fanRequested,appliance_.intervalEnergyWh()*3600,1,{});
        } else basement_=simulation::BasementModel({},t,rh);
        now_+=1000; refresh();
        if(now_%60000==0) sample();
    }
}
void DisplaySession::reboot() {
    const auto c=configuration();
    simulation::DehumidifierParameters p;
    p.initialTankLevelLiters=appliance_.tankLevelLiters();
    appliance_=simulation::DehumidifierModel(p);
    controller_=DryingController(c); now_=0; history_.clear(); refresh(); sample();
}
void DisplaySession::emptyTank() {
    appliance_.emptyTank(); controller_.resetTankEstimate();
    if(fault_==Fault::FullTank) fault_=Fault::None;
    refresh();
}
void DisplaySession::preset(unsigned i) {
    *this=DisplaySession();
    switch(i) {
    case 1: setClimate(15,82,30,75); break;
    case 2: setClimate(8,85,4,90); break;
    case 3: setClimate(15,62,5,90); break;
    case 4: setFault(Fault::BasementSensor); break;
    case 5: setClimate(15,82,30,75); advance(720); setFault(Fault::FullTank); advance(121); break;
    case 6: setClimate(15,82,30,75); { auto c=configuration(); c.dehumidifierAutoRestartSupported=false; configure(c); } break;
    case 7: setClimate(15,82,30,75); setFault(Fault::UnknownFeedback); advance(720); break;
    default: break;
    }
}
}
