#pragma once
#include "ClimateData.h"
#include <limits>
namespace ventilation {
// Independent low-voltage float input, never a proof of compressor operation.
// Disabled is optional; enabled but stale/invalid inhibits the compressor.
struct TankReading {
    bool enabled=false, full=false, valid=false;
    Timestamp measuredAtMs=0;
    bool fresh(Timestamp now, Timestamp timeout) const {
        return valid && measuredAtMs<=now && now-measuredAtMs<timeout;
    }
};
enum class DehumidifierFeedback { Unknown, Off, Starting, Running, NoPowerDraw, Fault };
struct DehumidifierReading {
    DehumidifierFeedback feedback = DehumidifierFeedback::Unknown;
    double powerWatts = std::numeric_limits<double>::quiet_NaN();
    Timestamp measuredAtMs = 0;
    bool valid = false;
    bool fresh(Timestamp now, Timestamp timeout) const {
        return valid && measuredAtMs <= now && now-measuredAtMs < timeout;
    }
};
inline const char* feedbackName(DehumidifierFeedback f) {
    switch(f) {
    case DehumidifierFeedback::Unknown: return "UNKNOWN";
    case DehumidifierFeedback::Off: return "OFF";
    case DehumidifierFeedback::Starting: return "STARTING";
    case DehumidifierFeedback::Running: return "RUNNING";
    case DehumidifierFeedback::NoPowerDraw: return "NO_POWER_DRAW";
    case DehumidifierFeedback::Fault: return "FAULT";
    }
    return "UNKNOWN";
}
}
