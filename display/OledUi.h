#pragma once
#include "DisplaySession.h"
#include <array>
#include <string>
namespace ventilation::display {
// Six rows, at most 25 monospace glyphs each, for the 128x64 SH1106 plan.
class OledUi {
public:
    unsigned page() const { return page_; }
    bool editing() const { return editing_; }
    void next(DisplaySession& session);
    void select(DisplaySession& session);
    std::array<std::wstring,6> rows(const DisplaySession& session) const;
private:
    unsigned page_=0;
    bool editing_=false;
    double draft_=0;
    unsigned modeDraft_=0;
};
}
