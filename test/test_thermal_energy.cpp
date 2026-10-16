#include "simulator/BasementModel.h"
#include "simulator/DehumidifierModel.h"
#include "core/Psychrometrics.h"
#include "Test.h"
using namespace ventilation;
using namespace ventilation::simulation;
int main() { return run([] {
    ThermalParameters thermal;
    BasementModel slow({26.4,100,0},14,40),airOnly({26.4,100,0},14,40);
    const ClimateReading outside{5,90,0,true};
    const double initialWater=slow.vaporMassGram();
    const double airCapacity=26.4*1.2*1005;
    NEAR(airCapacity,31838.4,1e-9);
    slow.evolveTemperature(outside,true,0,3600,thermal);
    auto fast=thermal; fast.effectiveThermalCapacityJPerK=0;
    airOnly.evolveTemperature(outside,true,0,3600,fast);
    CHECK(slow.temperatureC()>airOnly.temperatureC()+2);
    const double h=35+100.0/3600*1.2*1005;
    const double equilibrium=(35*14+(h-35)*5)/h;
    const double expected=equilibrium+(14-equilibrium)*std::exp(-h*3600/(3000000+airCapacity));
    NEAR(slow.temperatureC(),expected,1e-10);
    NEAR(slow.vaporMassGram(),initialWater,1e-9);
    const auto& balance=slow.thermalBalance();
    NEAR(balance.sensibleChangeJ+balance.wallHeatLossJ+balance.ventilationHeatLossJ,balance.suppliedHeatJ,1e-6);
    BasementModel heated({26.4,100,0},14,40);
    heated.evolveTemperature(outside,false,300,3600,thermal);
    CHECK(heated.temperatureC()>14);
    const auto& hb=heated.thermalBalance();
    NEAR(hb.suppliedHeatJ,1080000,1e-7);
    NEAR(hb.sensibleChangeJ+hb.wallHeatLossJ+hb.ventilationHeatLossJ,hb.suppliedHeatJ,1e-6);
    // Reproducer: endpoint power would charge 5 Wh instead of the actual partial startup energy.
    DehumidifierParameters p; p.minimumOffTime=0; p.startupDelay=30;
    BasementModel b({26.4,100,0},15,80); DehumidifierModel d(p);
    d.step(true,b,60);
    NEAR(d.effectiveExtractionLitersPerHour(),0.21,1e-12);
    const double saturated=b.volumeM3()*ventilation::psychrometrics::absoluteHumidityGm3(15,100);
    const double z0=saturated*(80-35)/100,scale=saturated*(80-35)/100;
    const double k=210/std::pow(scale,1.5);
    const double zAfter=std::pow(std::pow(z0,-0.5)+0.5*k*30/3600,-2);
    NEAR(d.totalExtractedLiters(),(z0-zAfter)/1000,1e-12);
    NEAR(d.intervalEnergyWh(),(300.0*30+2.0*30)/3600,1e-12);
    CHECK(d.intervalEnergyWh()<d.powerWatts()*60/3600);
    NEAR(p.extractionFactor(15,80),1,1e-12);
    CHECK(p.extractionFactor(15,65)<0.6);
    CHECK(p.extractionFactor(10,80)<1);
    NEAR(p.extractionFactor(5,80),0,0);
    p.initialTankLevelLiters=4.999;
    DehumidifierModel tank(p); BasementModel tankAir({26.4,100,0},15,80);
    tank.step(true,tankAir,60);
    CHECK(tank.tankFull() && !tank.compressorRunning() && tank.tankFullEvents()==1);
    const double fillHours=(std::pow(z0-1,-0.5)-std::pow(z0,-0.5))/(0.5*k);
    NEAR(tank.firstTankFullSeconds(),30+fillHours*3600,1e-8);
    tank.step(true,tankAir,60); CHECK(tank.tankFullEvents()==1);
    NEAR(tank.intervalEnergyWh(),2.0*60/3600,1e-12);
    tank.emptyTank(); NEAR(tank.emptiedLiters(),5,1e-12);
    bool rejected=false;
    try { heated.evolveTemperature(outside,false,1e12,3600,thermal); }
    catch(const std::runtime_error&) { rejected=true; }
    CHECK(rejected); // No silent temperature clamping / disappearance of energy.
}); }
