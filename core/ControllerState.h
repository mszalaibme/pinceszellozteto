#pragma once
namespace ventilation {
enum class Mode { Auto, On, Off };
enum class ControllerState {
    Drying, HumidityOK, OutdoorAirTooHumid, BasementTooCold, SensorError,
    MinimumOnTimer, MinimumOffTimer, ManualOn, ManualOff, ClockError
};
const char* stateName(ControllerState state);
struct ControllerOutput {
    bool fanRequested = false;
    ControllerState state = ControllerState::HumidityOK;
    const char* reason = "Humidity OK";
    bool sensorWarning = false;
    bool highHumidityWarning = false;
    bool dehumidificationSuggested = false;
    double differenceGm3 = 0;
};
}
