#include "display/DisplaySession.h"
#include <iostream>
#include <stdexcept>
void require(bool b) {if(!b)throw std::runtime_error("Tank interlock regression");}
int main() {
    using namespace ventilation;
    using namespace ventilation::display;
    DisplaySession s;
    s.preset(1); s.advance(660);
    if(!s.output().dehumidifierRequested) return 2;
    s.setFault(Fault::FullTank);
    if(s.output().dehumidifierRequested) {
        std::cerr<<"FAIL: full tank leaves dehumidifier commanded ON during minimum ON\n";
        return 1;
    }
    require(s.output().reasonCode==ReasonCode::TankFull);
    s.setMode(Mode::ManualDehumidifier);require(!s.output().dehumidifierRequested);
    s.reboot();require(!s.output().dehumidifierRequested && s.tankReading().full);
    s.emptyTank();s.advance(599);require(!s.output().dehumidifierRequested);
    s.advance(1);require(s.output().dehumidifierRequested);
    s.setFault(Fault::TankSensor);require(!s.output().dehumidifierRequested && s.output().reasonCode==ReasonCode::TankSensorFailure);
    s.setMode(Mode::ManualDehumidifier);require(!s.output().dehumidifierRequested);
    s.preset(0);s.setFault(Fault::FullTank);s.advance(300);require(s.output().fanRequested && !s.output().dehumidifierRequested);
    // Stale, future, invalid input and known full override a contradictory Running feedback.
    for(unsigned kind=0;kind<4;++kind) {
        Configuration c;c.dehumidifierAutoRestartSupported=true;c.dehumidifierMinimumOffTime=0;
        DryingController controller(c);
        TankReading input{true,false,true,1000000};
        if(kind==0)input.measuredAtMs=0;
        if(kind==1)input.measuredAtMs=1000001;
        if(kind==2)input.valid=false;
        if(kind==3)input.full=true;
        auto out=controller.update({15,85,1000000,true},{30,75,1000000,true},{DehumidifierFeedback::Running,350,1000000,true},1000000,input);
        require(!out.dehumidifierRequested);
        require(out.reasonCode==(kind==3?ReasonCode::TankFull:ReasonCode::TankSensorFailure));
    }
    std::cout<<"Full tank immediately inhibits dehumidifier\n";
}
