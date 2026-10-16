#pragma once
#include "core/DehumidifierState.h"
namespace ventilation {
struct ITankSensor {
    virtual ~ITankSensor() = default;
    // Adapter supplies debounced, supervised input; stale/invalid is fail-off.
    virtual TankReading read() = 0;
};
}
