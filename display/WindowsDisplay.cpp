#include <windows.h>
#include <shellapi.h>
#include <gdiplus.h>
#include "DisplaySession.h"
#include "DisplayStatus.h"
#include "OledUi.h"
#include "core/Psychrometrics.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace ventilation;
using namespace ventilation::display;
namespace {
constexpr COLORREF ink=RGB(30,58,52),muted=RGB(115,142,133),paper=RGB(242,244,237);
constexpr COLORREF lcd=RGB(13,30,29),card=RGB(24,45,42),white=RGB(232,243,232),green=RGB(156,224,174),amber=RGB(246,197,115),red=RGB(255,152,139);
DisplaySession session;
Configuration draft;
OledUi oled;
bool legacyScreen=true;
HWND monitorCheck=nullptr;
int page=0,settingsPage=0;
bool playing=false;
unsigned speed=10;
double windowScale=1;
HWND mainWindow=nullptr,editBT=nullptr,editBR=nullptr,editOT=nullptr,editOR=nullptr,presetBox=nullptr,faultBox=nullptr,dynamicCheck=nullptr,speedBox=nullptr,playButton=nullptr;
HFONT controlFont=nullptr;
std::vector<std::wstring> events;
std::wstring notice=L"A szimuláció szünetel. Indítsd az időt, vagy használd a +perc gombokat.";
std::string lastReason;
constexpr int sx=42,sy=120,sw=400,sh=533;

std::wstring number(double n,int precision=1) {
    if(!std::isfinite(n)) return L"—";
    std::wostringstream s; s<<std::fixed<<std::setprecision(precision)<<n; return s.str();
}
std::wstring clockText(double seconds) {
    const auto s=static_cast<unsigned long long>(seconds);
    std::wostringstream o; o<<std::setfill(L'0')<<std::setw(2)<<s/3600<<L":"<<std::setw(2)<<(s/60)%60<<L":"<<std::setw(2)<<s%60; return o.str();
}
void fill(HDC dc,int x,int y,int w,int h,COLORREF color,int radius=0) {
    HBRUSH brush=CreateSolidBrush(color); HGDIOBJ old=SelectObject(dc,brush); HGDIOBJ pen=SelectObject(dc,GetStockObject(NULL_PEN));
    if(radius) RoundRect(dc,x,y,x+w,y+h,radius,radius); else Rectangle(dc,x,y,x+w,y+h);
    SelectObject(dc,pen); SelectObject(dc,old); DeleteObject(brush);
}
void text(HDC dc,int x,int y,int w,int h,const std::wstring& value,int size,COLORREF color,bool bold=false,UINT align=DT_LEFT) {
    HFONT f=CreateFontW(-size,0,0,0,bold?FW_SEMIBOLD:FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
    auto old=SelectObject(dc,f); SetTextColor(dc,color); SetBkMode(dc,TRANSPARENT);
    RECT r{x,y,x+w,y+h}; DrawTextW(dc,value.c_str(),-1,&r,DT_NOPREFIX|DT_WORDBREAK|align);
    SelectObject(dc,old); DeleteObject(f);
}
void line(HDC dc,int x1,int y1,int x2,int y2,COLORREF color,int width=1) {
    HPEN p=CreatePen(PS_SOLID,width,color); auto old=SelectObject(dc,p); MoveToEx(dc,x1,y1,nullptr); LineTo(dc,x2,y2); SelectObject(dc,old); DeleteObject(p);
}
void button(HDC dc,int x,int y,int w,int h,const std::wstring& value,bool active=false,COLORREF accent=green) {
    fill(dc,x,y,w,h,active?accent:card,7); text(dc,x,y+(h-15)/2-2,w,h,value,12,active?lcd:white,true,DT_CENTER);
}
std::wstring reason() {
    auto value=describe(session).explanation;std::replace(value.begin(),value.end(),L'\n',L' ');return value;
}
std::wstring stateTitle() {
    auto value=describe(session).title;std::replace(value.begin(),value.end(),L'\n',L' ');return value;
}
void record() {
    if(lastReason!=session.output().reason) {
        lastReason=session.output().reason;
        events.insert(events.begin(),clockText(static_cast<double>(session.now())/1000)+L"   "+stateTitle()+L" / "+reason());
        if(events.size()>6) events.pop_back();
    }
}
void climateCard(HDC dc,int x,const std::wstring& title,ClimateReading reading) {
    fill(dc,x,58,145,99,card,10); text(dc,x+12,67,120,20,title,10,muted,true);
    const bool valid=reading.isPlausible();
    text(dc,x+12,87,128,31,valid?number(reading.temperatureC)+L" °C":L"— °C",25,white,true);
    text(dc,x+12,121,120,26,valid?number(reading.humidityRH,0)+L" %":L"NINCS ADAT",23,valid?green:red,true);
}
struct Setting { const wchar_t* name; double step,low,high; };
const std::array<Setting,18> settings{{
    {L"Cél páratartalom (%)",1,40,85},{L"RH indítási hiszterézis",1,1,15},{L"Magas RH riasztás (%)",1,50,100},
    {L"Venti indulás ΔAH",0.1,0.1,5},{L"Venti leállás ΔAH",0.1,0,4},{L"Minimum pince °C",0.5,5,20},
    {L"Venti min. BE (perc)",1,0,60},{L"Venti min. KI (perc)",1,0,60},{L"Páram. min. BE (perc)",1,0,60},
    {L"Páram. min. KI (perc)",1,0,60},{L"Indulási türelem (perc)",1,1,30},{L"Tartálybecslés méret (L)",0.5,0.5,20},
    {L"Hatástalanság (perc)",30,1,1440},{L"Auto restart igazolt",1,0,1},{L"Tartálybecslés BE",1,0,1},
    {L"Becsült kivonás (L/h)",0.01,0,1},{L"Minimum teljesítmény W",5,5,200},{L"Hatásfigyelés BE",1,0,1}
}};
double settingValue(int i) {
    switch(i) {
    case 0:return draft.targetBasementRH; case 1:return draft.rhStartHysteresis; case 2:return draft.emergencyRH;
    case 3:return draft.minVentilationHumidityAdvantage; case 4:return draft.stopVentilationHumidityAdvantage; case 5:return draft.minBasementTemperature;
    case 6:return static_cast<double>(draft.ventilationMinimumOnTime)/60000; case 7:return static_cast<double>(draft.ventilationMinimumOffTime)/60000;
    case 8:return static_cast<double>(draft.dehumidifierMinimumOnTime)/60000; case 9:return static_cast<double>(draft.dehumidifierMinimumOffTime)/60000;
    case 10:return static_cast<double>(draft.dehumidifierStartupGracePeriod)/60000; case 11:return draft.tankCapacityLiters;
    case 12:return static_cast<double>(draft.dehumidifierIneffectiveTimeout)/60000; case 13:return draft.dehumidifierAutoRestartSupported?1:0;
    case 14:return draft.tankEstimationEnabled?1:0; case 15:return draft.estimatedExtractionLitersPerHour;
    case 16:return draft.dehumidifierMinimumPowerWatts; default:return draft.performanceMonitorEnabled?1:0;
    }
}
void settingChange(int i,int direction) {
    const double v=std::clamp(settingValue(i)+settings[static_cast<size_t>(i)].step*direction,settings[static_cast<size_t>(i)].low,settings[static_cast<size_t>(i)].high);
    const auto ms=static_cast<Timestamp>(std::llround(v*60000));
    switch(i) {
    case 0:draft.targetBasementRH=v;break; case 1:draft.rhStartHysteresis=v;break; case 2:draft.emergencyRH=v;break;
    case 3:draft.minVentilationHumidityAdvantage=v;break;case 4:draft.stopVentilationHumidityAdvantage=v;break;case 5:draft.minBasementTemperature=v;break;
    case 6:draft.ventilationMinimumOnTime=ms;break;case 7:draft.ventilationMinimumOffTime=ms;break;
    case 8:draft.dehumidifierMinimumOnTime=ms;break;case 9:draft.dehumidifierMinimumOffTime=ms;break;
    case 10:draft.dehumidifierStartupGracePeriod=ms;break;case 11:draft.tankCapacityLiters=v;break;
    case 12:draft.dehumidifierIneffectiveTimeout=ms;break;case 13:draft.dehumidifierAutoRestartSupported=v>0;break;
    case 14:draft.tankEstimationEnabled=v>0;break;case 15:draft.estimatedExtractionLitersPerHour=v;break;
    case 16:draft.dehumidifierMinimumPowerWatts=v;break;default:draft.performanceMonitorEnabled=v>0;break;
    }
    notice=L"Szerkesztés · alkalmazáshoz MENTÉS";
}
void chart(HDC dc,int y,int height,bool humidity) {
    fill(dc,10,y,300,height,card,9);
    const auto& samples=session.history();
    if(samples.empty()) return;
    const double start=samples.front().seconds,end=std::max(start+60,samples.back().seconds);
    const auto mapY=[=](double value) { return y+height-17-static_cast<int>((value-(humidity?40:5))/(humidity?60:20)*(height-35)); };
    text(dc,21,y+8,255,18,humidity?L"RELATÍV PÁRATARTALOM · %":L"PINCEHŐMÉRSÉKLET · °C",10,muted,true);
    if(humidity) line(dc,22,mapY(session.configuration().targetBasementRH),297,mapY(session.configuration().targetBasementRH),muted);
    int px=0,py=0;
    for(const auto& s:samples) {
        int x=22+static_cast<int>((s.seconds-start)/(end-start)*275);
        int valueY=mapY(humidity?s.rh:s.temperature);
        if(px) line(dc,px,py,x,valueY,humidity?green:amber,2);
        px=x;py=valueY;
    }
}
void drawTouchScreen(HDC dc) {
    fill(dc,0,0,320,480,lcd);
    text(dc,13,12,210,24,L"PINCE / VEZÉRLŐ",12,white,true);
    text(dc,214,13,92,20,L"● SZIMULÁCIÓ",9,green,true,DT_RIGHT);
    text(dc,13,34,220,18,session.mode()==Mode::Auto?L"AUTOMATIKA AKTÍV · FIGYELI A PINCÉT":session.mode()==Mode::Off?L"AUTOMATIKUS SZÁRÍTÁS LETILTVA":L"KÉZI VEZÉRLÉS",9,muted);
    text(dc,239,34,67,18,L"Cél: "+number(session.configuration().targetBasementRH,0)+L"%",10,muted,false,DT_RIGHT);
    const auto& out=session.output();
    if(page==0) {
        climateCard(dc,10,L"PINCE",session.basement()); climateCard(dc,165,L"KINT",session.outside());
        const auto status=describe(session);
        const COLORREF accent=status.tone==StatusTone::Error?red:status.tone==StatusTone::Attention?amber:status.tone==StatusTone::Active?green:white;
        fill(dc,10,167,300,111,card,9);fill(dc,10,179,3,86,accent);
        text(dc,23,176,276,51,status.title,19,accent,true);
        text(dc,23,235,276,38,status.explanation,11,white);
        fill(dc,10,288,300,37,card,7);fill(dc,10,333,300,37,card,7);
        text(dc,21,299,132,21,L"Ventilátor",13,white,true);
        text(dc,150,299,148,21,status.fan,12,out.fanRequested?green:muted,true,DT_RIGHT);
        text(dc,21,344,132,21,L"Páramentesítő",13,white,true);
        text(dc,150,344,148,21,status.dehumidifier,12,status.dehumidifierConfirmedRunning?green:out.dehumidifierRequested?amber:muted,true,DT_RIGHT);
        const std::array<std::wstring,4> names{L"AUTO",L"VENTILÁTOR",L"PÁRAMENTESÍTŐ",L"KI"};
        const std::array<Mode,4> modes{Mode::Auto,Mode::ManualVentilation,Mode::ManualDehumidifier,Mode::Off};
        const int positions[]{10,66,158,270},widths[]{50,86,106,40};
        for(int i=0;i<4;++i) button(dc,positions[i],386,widths[i],33,names[static_cast<size_t>(i)],session.mode()==modes[static_cast<size_t>(i)]);
    } else if(page==1) {
        text(dc,15,64,290,28,L"PÁRAMENTESÍTŐ",19,white,true);
        const auto f=session.feedback();
        const auto row=[&](int y,const std::wstring& label,const std::wstring& value,COLORREF color) {
            text(dc,17,y,140,24,label,11,muted);text(dc,148,y-2,154,27,value,15,color,true,DT_RIGHT);
            line(dc,17,y+28,303,y+28,card);
        };
        row(111,L"Bekapcsolás kérése",out.dehumidifierRequested?L"BEKAPCSOLVA":L"KIKAPCSOLVA",out.dehumidifierRequested?green:muted);
        const auto status=describe(session);
        row(153,L"Készülék működése",status.dehumidifier,status.dehumidifierConfirmedRunning?green:amber);
        row(195,L"Mért teljesítmény",number(f.powerWatts,0)+L" W",white);
        row(237,L"Parancs üzemideje",clockText(out.dehumidifierRuntimeSeconds),white);
        const auto tank=session.tankReading();
        row(279,L"Tartály úszójele",!tank.valid?L"HIBÁS":tank.full?L"MEGTELT":L"NEM TELE",tank.full||!tank.valid?amber:green);
        text(dc,17,314,288,26,L"Egyszintű jelzés; nem mér százalékot.",10,amber);
        text(dc,17,347,286,34,L"Ürítsd ki a fizikai tartályt.\nAz úszójelet a kijelző nem törli.",11,muted);
        button(dc,14,391,292,29,L"MŰSZAKI ADATOK ›");
    } else if(page==2) {
        for(int row=0;row<6;++row) {
            const int index=settingsPage*6+row,y=61+row*47;
            text(dc,14,y,210,19,settings[static_cast<size_t>(index)].name,11,muted);
            const bool flag=index==13||index==14||index==17;
            text(dc,15,y+19,194,24,flag?(settingValue(index)>0?L"IGEN":L"NEM"):number(settingValue(index),index==15?2:1),16,white,true);
            button(dc,227,y+4,34,32,L"−");button(dc,271,y+4,34,32,L"+");
        }
        button(dc,14,350,60,29,L"‹");button(dc,246,350,60,29,L"›");
        text(dc,89,356,140,23,L"BEÁLLÍTÁSOK  "+std::to_wstring(settingsPage+1)+L" / 3",10,muted,true,DT_CENTER);
        button(dc,14,389,141,31,L"MENTÉS",true);button(dc,165,389,141,31,L"ALAPÉRTÉKEK");
    } else if(page==3) {
        text(dc,15,63,290,26,L"ELŐZMÉNYEK",19,white,true);
        chart(dc,103,143,true);chart(dc,260,125,false);
        text(dc,17,397,285,25,L"Percenkénti minta · legfeljebb 24 óra",10,muted);
    } else {
        text(dc,15,66,290,26,L"MŰSZAKI ADATOK",18,white,true);
        int y=110;
        for(const auto& entry:std::array<std::pair<std::wstring,ClimateReading>,2>{{{L"PINCE",session.basement()},{L"KINT",session.outside()}}}) {
            text(dc,17,y,280,20,entry.first,12,green,true);
            const auto p=entry.second.isPlausible()?psychrometrics::calculate(entry.second.temperatureC,entry.second.humidityRH):std::nullopt;
            text(dc,17,y+28,282,24,L"Abszolút nedvesség: "+(p?number(p->absoluteHumidityGm3):L"—")+L" g/m³",13,white);
            text(dc,17,y+57,282,23,L"Harmatpont: "+(p?number(p->dewPointC.value_or(NAN)):L"—")+L" °C",13,white);y+=105;
        }
        text(dc,17,331,285,25,L"Különbség: "+number(out.differenceGm3)+L" g/m³",14,white);
        text(dc,17,367,285,43,L"Ezekből számol a vezérlés. A főoldal\nközvetlenül a készülékek állapotát mutatja.",11,muted);
    }
    fill(dc,0,432,320,48,card);
    const std::array<std::wstring,4> tabs{L"FŐOLDAL",L"KÉSZÜLÉK",L"BEÁLLÍTÁS",L"ELŐZMÉNY"};
    for(int i=0;i<4;++i) {
        const bool active=i==page||(page==4&&i==1);
        if(active) fill(dc,i*80+12,432,56,3,green);
        text(dc,i*80,451,80,20,tabs[static_cast<size_t>(i)],9,active?green:muted,true,DT_CENTER);
    }
}
void drawScreen(HDC dc) {
    if(legacyScreen) {drawTouchScreen(dc);return;}
    fill(dc,0,0,320,480,lcd);
    text(dc,17,20,286,28,L"PINCE · OLED + 2 GOMB",17,white,true);
    text(dc,17,54,286,24,L"1,3 hüvelyk · 128 × 64 képpont",12,muted);
    HDC tiny=CreateCompatibleDC(dc);HBITMAP bitmap=CreateCompatibleBitmap(dc,128,64);
    auto old=SelectObject(tiny,bitmap);fill(tiny,0,0,128,64,RGB(0,0,0));
    HFONT font=CreateFontW(-9,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,FIXED_PITCH,L"Consolas");
    auto oldFont=SelectObject(tiny,font);SetTextColor(tiny,RGB(255,255,255));SetBkMode(tiny,TRANSPARENT);
    auto rows=oled.rows(session);
    for(int i=0;i<6;++i) TextOutW(tiny,1,1+i*10,rows[static_cast<size_t>(i)].c_str(),static_cast<int>(rows[static_cast<size_t>(i)].size()));
    SetStretchBltMode(dc,COLORONCOLOR);StretchBlt(dc,8,100,304,152,tiny,0,0,128,64,SRCCOPY);
    SelectObject(tiny,oldFont);DeleteObject(font);SelectObject(tiny,old);DeleteObject(bitmap);DeleteDC(tiny);
    text(dc,17,270,285,22,L"Oldal "+std::to_wstring(oled.page()+1)+L" / 6"+(oled.editing()?L" · SZERKESZTÉS":L""),12,muted);
    button(dc,17,310,169,46,L"KÖVETKEZŐ  >");button(dc,198,310,104,46,L"OK");
    text(dc,17,380,286,67,L"Nincs érintőfelület. A két gombbal lapozhatsz, módot választhatsz és beállíthatod a célértéket.\nBillentyűzet: → és Enter.",12,white);
}
void draw(HDC dc) {
    fill(dc,0,0,1100,820,paper);
    text(dc,38,24,700,30,L"Pince Lab",26,ink,true);
    text(dc,39,61,900,30,L"2,8 hüvelykes érintőkijelző · válassz módot közvetlenül a képernyőn.",15,muted);
    fill(dc,28,104,428,565,RGB(33,49,45),25);
    const int saved=SaveDC(dc);
    SetMapMode(dc,MM_ANISOTROPIC);SetWindowExtEx(dc,320,480,nullptr);SetViewportExtEx(dc,sw,sh,nullptr);SetViewportOrgEx(dc,sx,sy,nullptr);
    drawScreen(dc);RestoreDC(dc,saved);
    text(dc,45,685,405,60,L"ESP32-2432S028R-CYD · 240 × 320\n2,8 hüvelyk · érintős kezelés\nKülön OLED és kezelőgombok nélkül.",12,muted);
    text(dc,500,108,540,25,L"PRÓBAPAD",12,ink,true);
    text(dc,500,158,500,22,L"PINCE                             KÜLTÉR",12,muted,true);
    text(dc,500,190,66,24,L"°C",13,ink);text(dc,740,190,50,24,L"°C",13,ink);
    text(dc,500,230,66,24,L"RH %",13,ink);text(dc,740,230,50,24,L"RH %",13,ink);
    text(dc,500,303,490,24,L"HIBA / VISSZAJELZÉS",11,muted,true);
    text(dc,500,392,340,24,playing?L"SZIMULÁCIÓS IDŐ · HALAD":L"SZIMULÁCIÓS IDŐ · SZÜNETEL",11,muted,true);
    text(dc,848,392,182,22,clockText(static_cast<double>(session.now())/1000),12,ink,true,DT_RIGHT);
    text(dc,500,482,510,36,notice,12,ink);
    text(dc,500,529,510,24,L"DÖNTÉSI NAPLÓ",11,muted,true);
    for(size_t i=0;i<events.size();++i) text(dc,500,555+static_cast<int>(i)*27,530,27,events[i],11,i==0?ink:muted);
    std::wstring actual=L"Próbapad: valódi modell-tartály "+number(session.actualTank(),2)+L" / 5 L · kompresszor "+clockText(session.actualRuntime());
    text(dc,500,731,530,22,actual,11,ink);
    text(dc,500,760,520,43,L"Rögzített klíma: időzítők tesztje. Dinamikus klíma: víztömeg- és hőmodell, 100 g/h forrás. 300 W és 0,21 L/h feltételezés. A restart csak a szimulátorban igazolt.",10,muted);
}
HWND child(const wchar_t* cls,const wchar_t* label,DWORD style,int x,int y,int w,int h,int id) {
    const auto scaled=[](int v) { return static_cast<int>(std::lround(v*windowScale)); };
    HWND c=CreateWindowExW(cls==std::wstring(L"EDIT")?WS_EX_CLIENTEDGE:0,cls,label,WS_CHILD|WS_VISIBLE|WS_TABSTOP|style,scaled(x),scaled(y),scaled(w),scaled(h),mainWindow,reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),GetModuleHandleW(nullptr),nullptr);
    SendMessageW(c,WM_SETFONT,reinterpret_cast<WPARAM>(controlFont),TRUE);return c;
}
void syncInputs() {
    SetWindowTextW(editBT,number(session.basement().isPlausible()?session.basement().temperatureC:14.7).c_str());
    SetWindowTextW(editBR,number(session.basement().humidityRH).c_str());SetWindowTextW(editOT,number(session.outside().temperatureC).c_str());SetWindowTextW(editOR,number(session.outside().humidityRH).c_str());
    SendMessageW(faultBox,CB_SETCURSEL,static_cast<WPARAM>(session.fault()),0);
    SendMessageW(dynamicCheck,BM_SETCHECK,session.dynamic()?BST_CHECKED:BST_UNCHECKED,0);
}
double input(HWND c) {
    wchar_t s[80]{};GetWindowTextW(c,s,80);std::wstring value=s;
    std::replace(value.begin(),value.end(),L',',L'.');size_t count=0;double n=std::stod(value,&count);
    if(count!=value.size()) throw std::invalid_argument("Invalid number");return n;
}
void screenClick(int x,int y) {
    if(x<0||y<0||x>=320||y>=480) return;
    if(!legacyScreen) {
        if(y>=310 && y<=356) {
            if(x>=17 && x<=186) oled.next(session);
            else if(x>=198 && x<=302) oled.select(session);
        }
        return;
    }
    if(y>=432) { page=std::min(3,x/80);if(page==2) draft=session.configuration();return; }
    if(page==0 && y>=386 && y<=419) {
        const std::array<Mode,4> modes{Mode::Auto,Mode::ManualVentilation,Mode::ManualDehumidifier,Mode::Off};
        const int positions[]{10,66,158,270},widths[]{50,86,106,40};
        for(int i=0;i<4;++i) if(x>=positions[i]&&x<positions[i]+widths[i]) session.setMode(modes[static_cast<size_t>(i)]);
    }
    else if(page==1 && y>=391&&y<=420) page=4;
    else if(page==2) {
        if(y>=61&&y<343&&x>=227) { int r=(y-61)/47;settingChange(settingsPage*6+r,x<265?-1:1); }
        else if(y>=350&&y<379) { if(x<80) settingsPage=(settingsPage+2)%3;else if(x>240) settingsPage=(settingsPage+1)%3; }
        else if(y>=389&&y<=420) {
            if(x<160) {
                if(session.configure(draft)) notice=L"Beállítások alkalmazva az élő vezérlőn. NVS még nincs.";
                else notice=L"Érvénytelen: cél < riasztás, start ΔAH > stop ΔAH szükséges.";
            } else { draft=Configuration{};notice=L"Gyári alapérték-tervezet. Restart NEM igazolt. Mentsd az alkalmazáshoz."; }
        }
    }
}
void renderPng(const wchar_t* path) {
    HDC dc=CreateCompatibleDC(nullptr);BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=240;info.bmiHeader.biHeight=-320;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    void* bits=nullptr;HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);auto old=SelectObject(dc,bitmap);SetMapMode(dc,MM_ANISOTROPIC);SetWindowExtEx(dc,320,480,nullptr);SetViewportExtEx(dc,240,320,nullptr);drawScreen(dc);
    Gdiplus::Bitmap image(bitmap,nullptr);
    CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0x00,0x00,0xf8,0x1e,0xf3,0x2e}};
    const auto status=image.Save(path,&png,nullptr);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
    if(status!=Gdiplus::Ok) throw std::runtime_error("PNG export failed");
}
LRESULT CALLBACK windowProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg) {
    case WM_CREATE: {
        mainWindow=hwnd;controlFont=CreateFontW(-static_cast<int>(std::lround(16*windowScale)),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
        presetBox=child(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,610,104,420,270,100);
        const std::array<const wchar_t*,8> presets{L"Tél · ventilátor 5 perc múlva indulhat",L"Nyár · páramentesítő 10 perc múlva indulhat",L"Hideg pince · szellőztetés tiltva",L"Megfelelő páratartalom · készülékek kikapcsolva",L"Szenzorhiba · mindkét készülék leállítva",L"Tele tartály · úszó azonnal letiltja",L"Készülék újraindulása nincs igazolva",L"Nincs visszajelzés · működés nem igazolt"};
        for(auto p:presets) SendMessageW(presetBox,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(p));SendMessageW(presetBox,CB_SETCURSEL,0,0);
        editBT=child(L"EDIT",L"14.7",ES_AUTOHSCROLL,570,184,126,29,110);editBR=child(L"EDIT",L"78",ES_AUTOHSCROLL,570,224,126,29,111);
        editOT=child(L"EDIT",L"5.3",ES_AUTOHSCROLL,810,184,126,29,112);editOR=child(L"EDIT",L"91",ES_AUTOHSCROLL,810,224,126,29,113);
        child(L"BUTTON",L"Értékek alkalmazása",BS_PUSHBUTTON,500,264,260,30,114);
        child(L"BUTTON",L"Újraindítás / reboot",BS_PUSHBUTTON,775,264,255,30,115);
        faultBox=child(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,500,328,530,270,120);
        const std::array<const wchar_t*,10> faults{L"Nincs hiba",L"Pince szenzorhiba",L"Kültéri szenzorhiba",L"NaN pincehőmérséklet",L"Nincs érzékelt teljesítmény",L"Készülékhiba",L"Visszajelzés nem elérhető",L"Úszókapcsoló: TARTÁLY MEGTELT",L"Úszójel hibás / vezetékszakadás",L"Pince RS485 kapcsolat: nincs érvényes adat"};
        for(auto f:faults) SendMessageW(faultBox,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(f));SendMessageW(faultBox,CB_SETCURSEL,0,0);
        dynamicCheck=child(L"BUTTON",L"Dinamikus klíma (víztömeg + hőmodell)",BS_AUTOCHECKBOX,500,362,530,27,121);
        playButton=child(L"BUTTON",L"▶ Indítás",BS_PUSHBUTTON,500,418,130,32,130);
        speedBox=child(L"COMBOBOX",L"",CBS_DROPDOWNLIST,641,418,113,170,131);
        for(auto s:{L"1×",L"10×",L"60×",L"600×"}) SendMessageW(speedBox,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(s));SendMessageW(speedBox,CB_SETCURSEL,1,0);
        child(L"BUTTON",L"+1 perc",BS_PUSHBUTTON,766,418,81,32,132);child(L"BUTTON",L"+10 perc",BS_PUSHBUTTON,858,418,82,32,133);child(L"BUTTON",L"+1 óra",BS_PUSHBUTTON,950,418,80,32,134);
        child(L"BUTTON",L"Próbapadi tartály ürítése + becslés nullázása",BS_PUSHBUTTON,500,457,530,26,135);
        monitorCheck=child(L"BUTTON",L"Modell működésmérése (a kosárban nincs árammérő)",BS_AUTOCHECKBOX,500,690,530,25,140);

        SetTimer(hwnd,1,1000,nullptr);draft=session.configuration();record();return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);HDC back=CreateCompatibleDC(dc);HBITMAP bitmap=CreateCompatibleBitmap(dc,1100,820);auto old=SelectObject(back,bitmap);draw(back);RECT client{};GetClientRect(hwnd,&client);SetStretchBltMode(dc,HALFTONE);StretchBlt(dc,0,0,client.right,client.bottom,back,0,0,1100,820,SRCCOPY);SelectObject(back,old);DeleteObject(bitmap);DeleteDC(back);EndPaint(hwnd,&ps);return 0;
    }
    case WM_LBUTTONUP: {
        SetFocus(hwnd);
        int x=static_cast<int>(static_cast<short>(LOWORD(lp))/windowScale),y=static_cast<int>(static_cast<short>(HIWORD(lp))/windowScale);screenClick((x-sx)*320/sw,(y-sy)*480/sh);record();InvalidateRect(hwnd,nullptr,FALSE);return 0;
    }
    case WM_KEYDOWN:
        if(wp==VK_RIGHT) oled.next(session);else if(wp==VK_RETURN) oled.select(session);
        record();InvalidateRect(hwnd,nullptr,FALSE);return 0;
    case WM_TIMER: if(playing) { try { session.advance(speed);record();InvalidateRect(hwnd,nullptr,FALSE); } catch(...) { playing=false;SetWindowTextW(playButton,L"▶ Indítás");notice=L"A fizikai modell kilépett a támogatott tartományból. Válassz új próbát.";InvalidateRect(hwnd,nullptr,FALSE); } } return 0;
    case WM_COMMAND: {
        const int id=LOWORD(wp),notification=HIWORD(wp);
        try {
            if(id==100&&notification==CBN_SELCHANGE) { session.preset(static_cast<unsigned>(SendMessageW(presetBox,CB_GETCURSEL,0,0)));events.clear();lastReason.clear();draft=session.configuration();oled=OledUi{};SendMessageW(monitorCheck,BM_SETCHECK,BST_UNCHECKED,0);syncInputs();notice=L"Új próba betöltve; időzítők alaphelyzetben."; }
            else if(id==114) { session.setClimate(input(editBT),input(editBR),input(editOT),input(editOR));notice=L"Új klíma alkalmazva. A vezérlő időzítői tovább futnak."; }
            else if(id==115) { session.reboot();events.clear();lastReason.clear();notice=L"Reboot: AUTO, biztonságos indulás, időzítők újraindultak. Konfig RAM-ban marad."; }
            else if(id==120&&notification==CBN_SELCHANGE) { session.setFault(static_cast<Fault>(SendMessageW(faultBox,CB_GETCURSEL,0,0)));notice=L"Hiba alkalmazva. A türelmi időt az időgombokkal léptetheted."; }
            else if(id==121) { session.setDynamic(SendMessageW(dynamicCheck,BM_GETCHECK,0,0)==BST_CHECKED);notice=session.dynamic()?L"Dinamikus modell: az RH a vízgőztömegből származik.":L"Rögzített klíma: tesztbemenetek, nem fizikai vízmérleg."; }
            else if(id==130) { playing=!playing;SetWindowTextW(playButton,playing?L"Ⅱ Szünet":L"▶ Indítás"); }
            else if(id==131&&notification==CBN_SELCHANGE) { const unsigned speeds[]{1,10,60,600};speed=speeds[static_cast<size_t>(SendMessageW(speedBox,CB_GETCURSEL,0,0))]; }
            else if(id>=132&&id<=134) session.advance(id==132?60:id==133?600:3600);
            else if(id==140) { session.setPowerMonitor(SendMessageW(monitorCheck,BM_GETCHECK,0,0)==BST_CHECKED);notice=L"A működésmérés csak próbapadi modelladat; az úszó külön bemenet."; }
            else if(id==141) legacyScreen=SendMessageW(reinterpret_cast<HWND>(lp),BM_GETCHECK,0,0)==BST_CHECKED;
            else if(id==135) { session.emptyTank();syncInputs();notice=L"Próbapadi tartály ürítve; kijelzett becslés nullázva."; }
        } catch(const std::exception&) { notice=L"Érvénytelen adat. Pince: 5…25 °C, 40…100%; kint: −15…38 °C, 20…100%."; }
        record();InvalidateRect(hwnd,nullptr,FALSE);return 0;
    }
    case WM_DESTROY: KillTimer(hwnd,1);DeleteObject(controlFont);PostQuitMessage(0);return 0;
    default:return DefWindowProcW(hwnd,msg,wp,lp);
    }
}
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int show) {
    SetProcessDPIAware();
    ULONG_PTR gdip=0;Gdiplus::GdiplusStartupInput inputGdip;Gdiplus::GdiplusStartup(&gdip,&inputGdip,nullptr);
    int argc=0;LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    if(argc>=2 && std::wstring(argv[1])==L"--ui-test") {
        legacyScreen=true; bool ok=true;
        screenClick(115,400);ok=ok&&session.mode()==Mode::ManualVentilation&&session.output().fanRequested;
        screenClick(270,400);ok=ok&&session.mode()==Mode::Off&&!session.output().fanRequested;
        screenClick(200,454);ok=ok&&page==2;
        const double oldTarget=draft.targetBasementRH;
        screenClick(289,80);ok=ok&&draft.targetBasementRH==oldTarget+1&&session.configuration().targetBasementRH==oldTarget;
        screenClick(65,403);ok=ok&&session.configuration().targetBasementRH==oldTarget+1;
        screenClick(275,362);ok=ok&&settingsPage==1;
        screenClick(40,454);ok=ok&&page==0;
        screenClick(40,400);ok=ok&&session.mode()==Mode::Auto;
        session.preset(5);page=1;screenClick(100,360);ok=ok&&session.tankReading().full&&!session.output().dehumidifierRequested;
        legacyScreen=false;oled=OledUi{};
        screenClick(100,150);ok=ok&&oled.page()==0; // Display is not a touch input.
        for(int i=0;i<3;++i)screenClick(60,330);
        screenClick(250,330);screenClick(60,330);
        ok=ok&&oled.editing()&&session.mode()==Mode::Auto;
        screenClick(250,330);ok=ok&&!oled.editing()&&session.mode()==Mode::ManualVentilation;
        LocalFree(argv);Gdiplus::GdiplusShutdown(gdip);return ok?0:1;
    }
    if(argc>=3 && std::wstring(argv[1])==L"--render") {
        try { if(argc>=4) session.preset(static_cast<unsigned>(std::stoul(argv[3])));if(argc>=5) page=std::clamp(std::stoi(argv[4]),0,4);if(argc>=6) session.advance(static_cast<unsigned>(std::stoul(argv[5])));draft=session.configuration();renderPng(argv[2]); }
        catch(...) { LocalFree(argv);Gdiplus::GdiplusShutdown(gdip);return 1; }
        LocalFree(argv);Gdiplus::GdiplusShutdown(gdip);return 0;
    }
    LocalFree(argv);
    WNDCLASSW wc{};wc.lpfnWndProc=windowProc;wc.hInstance=instance;wc.lpszClassName=L"PinceDisplaySimulator";wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
    RECT work{};SystemParametersInfoW(SPI_GETWORKAREA,0,&work,0);
    windowScale=std::min({1.0,static_cast<double>(work.right-work.left-40)/1100,static_cast<double>(work.bottom-work.top-70)/820});
    RegisterClassW(&wc);RECT r{0,0,static_cast<LONG>(std::lround(1100*windowScale)),static_cast<LONG>(std::lround(820*windowScale))};AdjustWindowRect(&r,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,FALSE);
    HWND hwnd=CreateWindowExW(0,wc.lpszClassName,L"Pince Lab 0.7 · 2,8 hüvelykes ESP32 érintőkijelző",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,nullptr,nullptr,instance,nullptr);
    if(!hwnd) { Gdiplus::GdiplusShutdown(gdip);return 1; }
    ShowWindow(hwnd,show);UpdateWindow(hwnd);
    MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0) { if(!IsDialogMessageW(hwnd,&msg)) {TranslateMessage(&msg);DispatchMessageW(&msg);} }
    Gdiplus::GdiplusShutdown(gdip);return 0;
}
