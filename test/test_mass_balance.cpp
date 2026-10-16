#include "simulator/BasementModel.h"
#include "core/Psychrometrics.h"
#include "Test.h"
#include <random>
using namespace ventilation;
using namespace ventilation::simulation;
void conserved(const BasementModel& m) {
    const auto& b=m.balance();
    NEAR(m.vaporMassGram()+b.outgoingGram+b.condensedGram,
        b.initialVaporGram+b.generatedGram+b.incomingGram,1e-7);
    CHECK(m.vaporMassGram()>=0 && m.relativeHumidity()>=0 && m.relativeHumidity()<=100);
}
int main() { return run([] {
    // Isolated heating changes RH, never vapor mass or AH at fixed volume.
    ModelParameters sealed{26,100,0};
    BasementModel heat(sealed,14,60);
    const double mass=heat.vaporMassGram(), ah=heat.absoluteHumidityGm3();
    heat.setTemperatureC(25);
    NEAR(heat.vaporMassGram(),mass,1e-12); NEAR(heat.absoluteHumidityGm3(),ah,1e-12);
    CHECK(heat.relativeHumidity()<60); conserved(heat);
    heat.setTemperatureC(14); NEAR(heat.relativeHumidity(),60,1e-10);
    // Independent known source: 2 hours at 10 g/h adds exactly 20 g.
    BasementModel source({26,100,10},20,40);
    const double before=source.vaporMassGram();
    source.step({},false,7200); NEAR(source.vaporMassGram()-before,20,1e-9); conserved(source);
    // Air exchange solved analytically, with incoming air at indoor conditions.
    BasementModel winter(sealed,14,80);
    ClimateReading outside{5,90,0,true};
    const double initial=winter.vaporMassGram();
    const double rho=216.7*psychrometrics::vaporPressureHpa(5,90)/(273.15+14);
    winter.step(outside,true,3600);
    NEAR(winter.vaporMassGram(),26*rho+(initial-26*rho)*std::exp(-100.0/26),1e-9);
    CHECK(winter.relativeHumidity()<80); conserved(winter);
    // Cooling beyond saturation removes vapor to a separately accounted sink.
    BasementModel cool(sealed,25,90);
    const double water=cool.vaporMassGram(); cool.setTemperatureC(5);
    NEAR(cool.relativeHumidity(),100,1e-10); CHECK(cool.balance().condensedGram>0);
    NEAR(cool.vaporMassGram()+cool.balance().condensedGram,water,1e-9); conserved(cool);
    const double remaining=cool.vaporMassGram(); cool.setTemperatureC(25);
    NEAR(cool.vaporMassGram(),remaining,1e-12); // Condensate is removed, no invented re-evaporation.
    // Saturation reached during a step: source and outflow still balance exactly.
    BasementModel big({26,100,500},14,80), small({26,100,500},14,80);
    big.step(outside,true,3600);
    for(int i=0;i<3600;++i) small.step(outside,true,1);
    NEAR(big.vaporMassGram(),small.vaporMassGram(),1e-8);
    NEAR(big.balance().condensedGram,small.balance().condensedGram,1e-7);
    NEAR(big.balance().outgoingGram,small.balance().outgoingGram,1e-7);
    conserved(big); conserved(small);
    // 10,000 changing physical states: no direct RH updates, mass ledger closes.
    std::mt19937 rng(20261016);
    auto u=[&](double a,double b) { return a+(b-a)*static_cast<double>(rng())/4294967295.; };
    BasementModel evolving({26,100,25},14,80);
    for(int i=0;i<10000;++i) {
        evolving.setTemperatureC(u(5,25));
        evolving.step({u(-15,38),u(20,100),0,true},i%3!=0,u(1,300));
        conserved(evolving);
        NEAR(evolving.reading(123).humidityRH,evolving.relativeHumidity(),1e-12);
    }
    auto invalid=[&](auto operation) { bool threw=false; try { operation(); }
        catch(const std::invalid_argument&) { threw=true; } CHECK(threw); };
    invalid([&] { evolving.step(outside,true,NAN); });
    invalid([&] { evolving.step(outside,true,-1); });
    invalid([&] { evolving.step({},true,1); });
    invalid([&] { evolving.setTemperatureC(INFINITY); });
    invalid([] { BasementModel bad({0,100,25}); });
    const double unchanged=evolving.vaporMassGram(); evolving.step(outside,true,0);
    NEAR(evolving.vaporMassGram(),unchanged,0); conserved(evolving);
}); }
