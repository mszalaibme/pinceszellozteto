#include "ControllerState.h"
namespace ventilation {
const char* stateName(ControllerState s) {
    switch(s) {
    case ControllerState::Drying: return "DRYING";
    case ControllerState::HumidityOK: return "HUMIDITY_OK";
    case ControllerState::OutdoorAirTooHumid: return "OUTDOOR_AIR_TOO_HUMID";
    case ControllerState::BasementTooCold: return "BASEMENT_TOO_COLD";
    case ControllerState::SensorError: return "SENSOR_ERROR";
    case ControllerState::MinimumOnTimer: return "MINIMUM_ON_TIMER";
    case ControllerState::MinimumOffTimer: return "MINIMUM_OFF_TIMER";
    case ControllerState::ManualOn: return "MANUAL_ON";
    case ControllerState::ManualOff: return "MANUAL_OFF";
    case ControllerState::ClockError: return "CLOCK_ERROR";
    case ControllerState::Idle: return "IDLE";
    case ControllerState::Ventilating: return "VENTILATING";
    case ControllerState::Dehumidifying: return "DEHUMIDIFYING";
    case ControllerState::ManualVentilation: return "MANUAL_VENTILATION";
    case ControllerState::ManualDehumidifier: return "MANUAL_DEHUMIDIFIER";
    case ControllerState::DehumidifierError: return "DEHUMIDIFIER_ERROR";
    case ControllerState::WaitingMinimumOffTime: return "WAITING_MINIMUM_OFF_TIME";
    }
    return "UNKNOWN";
}
}
