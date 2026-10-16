#include "OledUi.h"
#include "DisplayStatus.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>
namespace ventilation::display {
namespace {
std::wstring n(double v,int p=0) {
    if(!std::isfinite(v)) return L"--";
    std::wostringstream o;o<<std::fixed<<std::setprecision(p)<<v;return o.str();
}
const std::array<Mode,4> modes{Mode::Auto,Mode::ManualVentilation,Mode::ManualDehumidifier,Mode::Off};
const std::array<std::wstring,4> names{L"AUTOMATIKUS",L"KÉZI VENTILÁTOR",L"KÉZI PÁRAMENTESÍTŐ",L"KIKAPCSOLVA"};
unsigned modeIndex(Mode m) { for(unsigned i=0;i<modes.size();++i) if(m==modes[i])return i;return 0; }
std::vector<std::wstring> wrap(std::wstring value) {
    for(auto& c:value) if(c==L'\n') c=L' ';
    std::wistringstream in(value);std::wstring word,line;std::vector<std::wstring> lines;
    while(in>>word) {
        if(!line.empty() && line.size()+word.size()+1>25) {lines.push_back(line);line.clear();}
        if(!line.empty()) line+=L' ';line+=word;
    }
    if(!line.empty()) lines.push_back(line);return lines;
}
}
void OledUi::next(DisplaySession&) {
    if(!editing_) {page_=(page_+1)%6;return;}
    if(page_==3) modeDraft_=(modeDraft_+1)%modes.size();
    if(page_==4) {draft_+=1;if(draft_>85)draft_=40;}
    if(page_==5) {draft_+=0.5;if(draft_>20)draft_=5;}
}
void OledUi::select(DisplaySession& s) {
    if(page_<3) return; // No acknowledgement may clear a physical full switch.
    if(!editing_) {
        editing_=true;modeDraft_=modeIndex(s.mode());
        draft_=page_==4?s.configuration().targetBasementRH:s.configuration().minBasementTemperature;
        return;
    }
    if(page_==3) {s.setMode(modes[modeDraft_]);editing_=false;return;}
    auto c=s.configuration();
    if(page_==4) c.targetBasementRH=draft_;else c.minBasementTemperature=draft_;
    if(s.configure(c)) editing_=false;
}
std::array<std::wstring,6> OledUi::rows(const DisplaySession& s) const {
    std::array<std::wstring,6> r{};
    const auto& o=s.output();const auto tank=s.tankReading();
    auto climate=[](const wchar_t* label,const ClimateReading& v) {
        return std::wstring(label)+(v.isPlausible()?n(v.temperatureC,1)+L"°C "+n(v.humidityRH)+L"%":L"NINCS ADAT");
    };
    if(page_==0) {
        r[0]=s.mode()==Mode::Auto?L"AUTO: FIGYELI A PINCÉT":names[modeIndex(s.mode())];
        r[1]=climate(L"Pince: ",s.basement());r[2]=climate(L"Kint:  ",s.outside());
        r[3]=o.fanRequested?L"Ventilátor: BE":L"Ventilátor: KI";
        r[4]=o.dehumidifierRequested?L"Párament. táp: BE *":L"Párament. táp: KI";
        r[5]=!tank.valid?L"TARTÁLYJEL HIBÁS":tank.full?L"TARTÁLY MEGTELT!":o.sensorWarning?L"ÉRZÉKELŐHIBA!":L"KÖVETKEZŐ: miért? >";
    } else if(page_==1) {
        r[0]=L"MI TÖRTÉNIK?";
        const auto status=describe(s);
        auto lines=wrap(status.explanation);
        for(size_t i=0;i<5 && i<lines.size();++i) r[i+1]=lines[i];
    } else if(page_==2) {
        r[0]=L"TARTÁLY / VISSZAJELZÉS";
        r[1]=!tank.valid?L"Úszójel: HIBÁS":tank.full?L"Úszójel: MEGTELT":L"Úszójel: NEM TELE";
        r[2]=L"Százalék nem mérhető.";
        r[3]=s.powerMonitor()?L"Működés: modelladat":L"Működés: NEM IGAZOLT";
        r[4]=L"* BE = áramot kap.";
        r[5]=tank.full?L"Ürítsd ki a tartályt!":L"Az úszó nem árammérő.";
    } else {
        r[0]=page_==3?L"ÜZEMMÓD":page_==4?L"CÉL PÁRATARTALOM":L"MINIMUM PINCEHŐMÉRSÉKLET";
        r[1]=page_==3?names[editing_?modeDraft_:modeIndex(s.mode())]:n(editing_?draft_:page_==4?s.configuration().targetBasementRH:s.configuration().minBasementTemperature,page_==5?1:0)+(page_==4?L" %":L" °C");
        r[3]=editing_?L"KÖVETKEZŐ: változtat":L"OK: szerkesztés";
        r[4]=editing_?L"OK: mentés / alkalmazás":L"KÖVETKEZŐ: másik oldal";
        r[5]=L"Mentés itt: RAM (próba)";
        if(page_==4 && editing_ && draft_>=s.configuration().emergencyRH) r[2]=L"Hibás: cél >= riasztás";
    }
    return r;
}
}
