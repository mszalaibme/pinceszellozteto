#pragma once
#include "core/DryingController.h"
#include "simulator/BasementModel.h"
#include "simulator/DehumidifierModel.h"
#include <deque>
namespace ventilation::display {
enum class Fault { None, BasementSensor, OutdoorSensor, NaN, NoPower, Appliance, UnknownFeedback, FullTank, TankSensor, BasementLink };
struct Sample { double seconds, rh, temperature; bool fan, dehumidifier; };
// Interactive boundary conditions are separate from the shared production controller.
class DisplaySession {
public:
    DisplaySession();
    const Configuration& configuration() const { return controller_.configuration(); }
    bool configure(const Configuration& c);
    void setMode(Mode mode);
    Mode mode() const { return controller_.mode(); }
    void setClimate(double basementT,double basementRH,double outsideT,double outsideRH);
    void setFault(Fault f);
    Fault fault() const { return fault_; }
    void setDynamic(bool value) { dynamic_=value; }
    bool dynamic() const { return dynamic_; }
    void advance(unsigned seconds);
    void reboot();
    void emptyTank();
    void preset(unsigned index);
    void refresh();
    Timestamp now() const { return now_; }
    ClimateReading basement() const;
    ClimateReading outside() const;
    DehumidifierReading feedback() const;
    TankReading tankReading() const;
    void setPowerMonitor(bool enabled) { powerMonitor_=enabled; refresh(); }
    bool powerMonitor() const { return powerMonitor_; }
    const ControllerOutput& output() const { return output_; }
    double actualTank() const;
    double actualRuntime() const { return appliance_.runtimeSeconds(); }
    const std::deque<Sample>& history() const { return history_; }
private:
    DryingController controller_;
    simulation::BasementModel basement_;
    simulation::DehumidifierModel appliance_;
    ControllerOutput output_;
    double outsideT_=5.3,outsideRH_=91;
    Timestamp now_=0;
    Fault fault_=Fault::None;
    bool dynamic_=false;
    bool powerMonitor_=false;
    std::deque<Sample> history_;
    void sample();
};
}
