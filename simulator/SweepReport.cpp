#include "BatchSimulation.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
namespace ventilation::simulation {
namespace {
struct Series { std::string label,color; std::vector<std::pair<double,double>> points; };
void plot(std::ostream& f,const char* title,const char* xLabel,const char* yLabel,std::vector<Series> lines) {
    double xmin=std::numeric_limits<double>::infinity(),xmax=-xmin,ymin=xmin,ymax=-xmin;
    for(auto& line:lines) { std::sort(line.points.begin(),line.points.end()); for(const auto& p:line.points) {
        xmin=std::min(xmin,p.first); xmax=std::max(xmax,p.first); ymin=std::min(ymin,p.second); ymax=std::max(ymax,p.second);
    }}
    f<<"<section><h2>"<<title<<"</h2>";
    if(!std::isfinite(xmin)) { f<<"<p>Nincs összehasonlítható futás.</p></section>"; return; }
    const double xpad=std::max(0.01,(xmax-xmin)*0.06),ypad=std::max(0.1,(ymax-ymin)*0.1);
    xmin-=xpad; xmax+=xpad; ymin-=ypad; ymax+=ypad;
    auto x=[&](double v){return 55+(v-xmin)/(xmax-xmin)*465;};
    auto y=[&](double v){return 230-(v-ymin)/(ymax-ymin)*195;};
    f<<std::setprecision(5)<<"<svg viewBox='0 0 550 285' role='img' aria-label='"<<title<<"'>";
    for(int i=0;i<=4;++i) {
        const double a=xmin+(xmax-xmin)*i/4,b=ymin+(ymax-ymin)*i/4;
        f<<"<path d='M55 "<<y(b)<<" H520' stroke='#e1e8e1'/><text x='8' y='"<<y(b)+4<<"'>"<<b<<"</text><text x='"<<x(a)-10<<"' y='250'>"<<a<<"</text>";
    }
    for(const auto& line:lines) {
        f<<"<polyline fill='none' stroke='"<<line.color<<"' stroke-width='2' points='";
        for(const auto& p:line.points) f<<x(p.first)<<','<<y(p.second)<<' ';
        f<<"'/>";
        for(const auto& p:line.points) f<<"<circle cx='"<<x(p.first)<<"' cy='"<<y(p.second)<<"' r='3.5' fill='"<<line.color<<"'/>";
    }
    f<<"<text x='230' y='277'>"<<xLabel<<"</text><text x='10' y='16'>"<<yLabel<<"</text></svg><p class='legend'>";
    for(const auto& line:lines) f<<"<span style='color:"<<line.color<<"'>● "<<line.label<<"</span> ";
    f<<"</p></section>";
}
}
void writeSweepReport(const std::vector<std::pair<std::string,SimulationResult>>& results,const std::filesystem::path& path) {
    std::ofstream f(path); if(!f) throw std::runtime_error("Cannot write sweep report");
    f<<R"HTML(<!doctype html><html lang="hu"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Pince · Kalibráció és érzékenység</title><style>
body{font:14px/1.6 system-ui,Segoe UI,sans-serif;color:#173b30;background:#f3f4ef;margin:0}header{background:#173b30;color:#eef5ee;padding:28px 5vw}h1{font-size:27px;margin:4px 0}header small{letter-spacing:2px;color:#a6ccb3}main{max-width:1260px;padding:25px 4vw;margin:auto}a{color:#147d64}a.button{display:inline-block;background:#fff;padding:10px 16px;border:1px solid #cedbce;border-radius:7px;margin:0 10px 14px 0;text-decoration:none}.notice{background:#fff0d7;border-left:4px solid #c88a2d;padding:16px 20px;margin:8px 0 22px}.plots{display:grid;grid-template-columns:1fr 1fr;gap:18px}section{background:white;border:1px solid #dce4dc;border-radius:10px;padding:17px}h2{font-size:16px;margin:0 0 10px}svg{width:100%;height:auto}svg text{font:11px Segoe UI,sans-serif;fill:#64766a}.legend{font-size:12px;margin:0}.legend span{margin-right:15px}.tablewrap{overflow:auto;background:white;border:1px solid #dce4dc;border-radius:10px;margin:20px 0;max-height:600px}table{border-collapse:collapse;width:100%;font-size:12px;white-space:nowrap}th,td{padding:9px 12px;border-bottom:1px solid #e7ece7;text-align:right}th{background:#e6eee6;position:sticky;top:0}td:first-child,th:first-child{text-align:left}.note{color:#65766b;font-size:12px}@media(max-width:750px){.plots{grid-template-columns:1fr}h1{font-size:21px}}</style><header><small>PINCE LAB / 0.3.0 · WINDOWS</small><h1>Hét nap, változó feltételekkel.</h1><div>Kalibrációs hipotézisek és paraméter-érzékenység · gyári specifikáció feltételezése nélkül</div></header><main><a class="button" href="report.html">Idősorok és lejátszás ↗</a><a class="button" href="sensitivity-summary.csv">Összes eredmény CSV ↗</a><div class="notice"><b>SIMULATION ASSUMPTION / NEEDS HARDWARE VERIFICATION</b><br>A 0.21 L/h empirikus referenciapont, nem névleges kapacitás. A 300 W és a termikus paraméterek feltételezések. Ismeretlen ventilátorfogyasztás esetén az energia a páramentesítőre elszámolt részösszeg: a 0 kWh nem nulla rendszerfogyasztás. A kalibrációs példa nem igazolja a teljes készülékgörbét vagy a valódi falnedvességet.</div><div class="plots">)HTML";
    Series mean{"Átlag RH","#147d64",{}},max{"Maximum RH","#cf8c32",{}};
    Series extraction{"Átlag RH · nedves kültér, 24 h ürítés","#147d64",{}};
    Series wetPower{"Nedves kültér","#cf8c32",{}},winterPower{"Téli baseline","#147d64",{}};
    Series flow60{"60 m³/h","#547da4",{}},flow80{"80 m³/h","#8c6fa3",{}},flow100{"100 m³/h","#147d64",{}},flow120{"120 m³/h","#cf8c32",{}};
    Series thermal{"Fal/Talaj hőmérséklet · C=3 MJ/K, H=35 W/K","#147d64",{}};
    Series capacity{"Épületi hőkapacitás · fal=14°C, H=35 W/K","#547da4",{}};
    for(const auto& [group,r]:results) {
        const auto& c=r.configuration;
        if(group=="baseline") { mean.points.emplace_back(c.moistureIngressGPerHour,r.averageBasementRH); max.points.emplace_back(c.moistureIngressGPerHour,r.maximumBasementRH); }
        if(group=="extraction-wet") extraction.points.emplace_back(c.dehumidifier.referenceExtractionLh,r.averageBasementRH);
        if(group=="power-wet") wetPower.points.emplace_back(c.dehumidifier.powerWatts,r.cumulativeEnergyKWh);
        if(group=="power-baseline") winterPower.points.emplace_back(c.dehumidifier.powerWatts,r.cumulativeEnergyKWh);
        if(group=="factorial" && std::abs(c.dehumidifier.referenceExtractionLh-0.21)<1e-9 && c.controller.targetBasementRH==65 && c.controller.minBasementTemperature==10) {
            Series* series=c.ventilationAirflowM3h==60?&flow60:c.ventilationAirflowM3h==80?&flow80:c.ventilationAirflowM3h==100?&flow100:c.ventilationAirflowM3h==120?&flow120:nullptr;
            if(series) series->points.emplace_back(c.moistureIngressGPerHour,r.averageBasementRH);
        }
        if(group=="thermal" && c.thermal.wallAirHeatTransferWPerK==35) {
            if(c.thermal.effectiveThermalCapacityJPerK==3000000) thermal.points.emplace_back(c.thermal.wallTemperatureC,r.minimumBasementTemperature);
            if(c.thermal.wallTemperatureC==14) capacity.points.emplace_back(c.thermal.effectiveThermalCapacityJPerK/1e6,r.minimumBasementTemperature);
        }
    }
    plot(f,"Falnedvesség → pincei RH","Nedvességforrás [g/h]","RH [%]",{mean,max});
    plot(f,"Légszállítás érzékenysége · RH cél 65%, minimum 10°C","Nedvességforrás [g/h]","Átlag RH [%]",{flow60,flow80,flow100,flow120});
    plot(f,"Empirikus vízkivonás · külön nedves-kültéri vizsgálat","Referencia [L/h]","Átlag RH [%]",{extraction});
    plot(f,"Villamos teljesítmény · elszámolt energia, ventilátor nélkül","Feltételezett teljesítmény [W]","Energia [kWh]",{wetPower,winterPower});
    plot(f,"Környező szerkezet / talaj hőmérséklete","Fal/Talaj [°C]","Minimum pince T [°C]",{thermal});
    plot(f,"Épületi hőtárolás","Épületi hőkapacitás [MJ/K]","Minimum pince T [°C]",{capacity});
    f<<"</div><p class='note'>A metszetek feltételei a grafikonok címében szerepelnek; a teljes rács a táblában és CSV-ben található. Egy nem aktiválódó páramentesítő lapos teljesítménygörbéje nem bizonyít általános érzéketlenséget. A napi ürítés csak a külön nedves-kültéri csoportokra vonatkozik. A baseline és a teljes faktoros téli rács alapból nem ürít tartályt.</p><h2>Összes futás · "<<results.size()<<" sor</h2><div class='tablewrap'><table><thead><tr><th>Futás / csoport</th><th>Forrás g/h</th><th>Q m³/h</th><th>Ref. L/h</th><th>Cél RH</th><th>Min T küszöb</th><th>Átlag RH</th><th>Max RH</th><th>Min T</th><th>Venti h</th><th>Kompr. h</th><th>Venti L</th><th>Kivont L</th><th>Tele esemény</th><th>Elszámolt kWh*</th><th>&gt;70% h</th><th>&gt;80% h</th><th>Túl hideg h</th><th>Kondenzátum L</th></tr></thead><tbody>";
    f<<std::fixed<<std::setprecision(2);
    for(const auto& [group,r]:results) {
        const auto& c=r.configuration;
        f<<"<tr><td>"<<r.scenario.id<<" · "<<group<<"</td><td>"<<c.moistureIngressGPerHour<<"</td><td>"<<c.ventilationAirflowM3h
            <<"</td><td>"<<c.dehumidifier.referenceExtractionLh<<"</td><td>"<<c.controller.targetBasementRH<<"</td><td>"<<c.controller.minBasementTemperature
            <<"</td><td>"<<r.averageBasementRH<<"</td><td>"<<r.maximumBasementRH<<"</td><td>"<<r.minimumBasementTemperature
            <<"</td><td>"<<r.fanHours<<"</td><td>"<<r.compressorHours<<"</td><td>"<<r.netVentilationLiters<<"</td><td>"<<r.extractedLiters
            <<"</td><td>"<<r.tankFullEventCount<<"</td><td>"<<r.cumulativeEnergyKWh<<"</td><td>"<<r.timeAbove70RH<<"</td><td>"<<r.timeAbove80RH
            <<"</td><td>"<<r.timeBelowMinimumTemperature<<"</td><td>"<<r.condensedLiters<<"</td></tr>";
    }
    f<<"</tbody></table></div><p class='note'>* Ismeretlen ventilátorteljesítmény esetén részösszeg. A kondenzátum nem páramentesítő-teljesítmény és nem elfogadható szárítási eredmény. Ezek modellkísérletek, nem hardveres minősítések.</p></main></html>";
    if(!f) throw std::runtime_error("Sweep report export failed");
}
}
