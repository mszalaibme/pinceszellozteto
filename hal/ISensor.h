#pragma once
#include "core/ClimateData.h"
namespace ventilation {
struct ISensor { virtual ~ISensor() = default; virtual ClimateReading read() = 0; };
}
