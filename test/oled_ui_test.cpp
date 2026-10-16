#include "display/OledUi.h"
#include <stdexcept>
#include <iostream>
void require(bool b) {if(!b)throw std::runtime_error("OLED regression");}
int main() {
    using namespace ventilation;using namespace ventilation::display;
    DisplaySession s;OledUi ui;
    require(!s.powerMonitor());
    require(ui.rows(s)[3]==L"Ventilátor: KI");
    ui.select(s);require(s.mode()==Mode::Auto);
    for(unsigned i=0;i<3;++i)ui.next(s);
    ui.select(s);ui.next(s);require(s.mode()==Mode::Auto);
    ui.select(s);require(s.mode()==Mode::ManualVentilation);
    ui.next(s);const double target=s.configuration().targetBasementRH;
    ui.select(s);ui.next(s);require(s.configuration().targetBasementRH==target);
    ui.select(s);require(s.configuration().targetBasementRH==target+1);
    s.reboot();require(s.configuration().targetBasementRH==target+1 && s.mode()==Mode::Auto);
    ui.next(s);ui.select(s);ui.next(s);ui.select(s);
    require(s.configuration().minBasementTemperature==10.5);
    ui.next(s);require(ui.page()==0);
    // Every screen/preset must fit the physical frame; no hidden clipping.
    for(unsigned preset=0;preset<8;++preset) {
        s.preset(preset);OledUi screens;
        for(unsigned p=0;p<6;++p) {
            for(const auto& row:screens.rows(s))require(row.size()<=25);
            screens.next(s);
        }
    }
    s.preset(1);s.advance(660);require(s.output().dehumidifierRequested);
    require(!s.feedback().valid); // Cheap basket has no current monitor.
    s.setFault(Fault::BasementLink);
    require(s.output().sensorWarning && !s.output().fanRequested && !s.output().dehumidifierRequested);
    s.setFault(Fault::None);s.advance(600);
    ui=OledUi{};ui.next(s);ui.next(s);
    require(ui.rows(s)[3]==L"Működés: NEM IGAZOLT");
    s.setFault(Fault::FullTank);ui.select(s);require(s.tankReading().full && !s.output().dehumidifierRequested);
    std::cout<<"OLED: physical row limits, two buttons, staged mode/settings, reboot, independent full switch, unknown operation passed\n";
}
