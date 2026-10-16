#pragma once
#include "DisplaySession.h"
#include <string>
namespace ventilation::display {
enum class StatusTone { Normal, Active, Attention, Error };
struct DisplayStatus {
    std::wstring title, explanation, fan, dehumidifier;
    StatusTone tone=StatusTone::Normal;
    bool dehumidifierConfirmedRunning=false;
};
DisplayStatus describe(const DisplaySession& session);
}
