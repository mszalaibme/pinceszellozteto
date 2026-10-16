#pragma once
namespace ventilation {
enum class Mode { Auto, On, Off, ManualDehumidifier,
    ManualVentilation = On, ManualOff = Off };
enum class ControllerState {
    Drying, HumidityOK, OutdoorAirTooHumid, BasementTooCold, SensorError,
    MinimumOnTimer, MinimumOffTimer, ManualOn, ManualOff, ClockError,
    Idle, Ventilating, Dehumidifying, ManualVentilation, ManualDehumidifier,
    DehumidifierError, WaitingMinimumOffTime
};
enum class ReasonCode { HumidityOK, OutdoorAirDrier, DehumidifierPreferred, Manual,
    SensorFailure, TooCold, MinimumOn, MinimumOff, AutoRestartUnverified,
    NoPowerDraw, DeviceFault, ClockFailure };
const char* stateName(ControllerState state);
struct ControllerOutput {
    bool fanRequested = false;
    bool ventilationRequested = false;
    bool dehumidifierRequested = false;
    ControllerState state = ControllerState::HumidityOK;
    const char* reason = "Humidity OK";
    bool sensorWarning = false;
    bool highHumidityWarning = false;
    bool dehumidificationSuggested = false;
    double differenceGm3 = 0;
    ReasonCode reasonCode = ReasonCode::HumidityOK;
    bool feedbackUnavailable = false;
    bool dehumidifierIneffective = false;
    bool tankEstimateAvailable = false;
    double estimatedTankLevelLiters = 0;
    double dehumidifierRuntimeSeconds = 0;
};
}
