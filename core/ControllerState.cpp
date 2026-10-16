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
    }
    return "UNKNOWN";
}
}
