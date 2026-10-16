#pragma once
#include "core/ClimateData.h"
namespace ventilation {
struct IClock { virtual ~IClock() = default; virtual Timestamp nowMs() const = 0; };
}
