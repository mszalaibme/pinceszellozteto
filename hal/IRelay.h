#pragma once
namespace ventilation {
struct IRelay { virtual ~IRelay() = default; virtual void setRequested(bool on) = 0; };
}
