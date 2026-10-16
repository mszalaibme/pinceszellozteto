#pragma once
#include "core/DehumidifierState.h"
namespace ventilation {
struct IDehumidifierMonitor {
    virtual ~IDehumidifierMonitor() = default;
    virtual DehumidifierReading read() = 0;
};
}
