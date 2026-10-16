#include "simulator/DehumidifierModel.h"
#include "Test.h"
using namespace ventilation;
using namespace ventilation::simulation;
int main() { return run([] {
    DehumidifierParameters p; p.minimumOffTime=10; p.startupDelay=2;
    BasementModel b({26,100,250},20,90);
    DehumidifierModel d(p);
    d.step(true,b,5); CHECK(!d.compressorRunning());
    d.step(true,b,6); CHECK(d.compressorRunning());
    CHECK(d.totalExtractedLiters()>0);
    const auto& balance=b.balance();
    NEAR(balance.extractedGram,d.totalExtractedLiters()*1000,1e-8);
    NEAR(b.vaporMassGram()+balance.extractedGram,balance.initialVaporGram,1e-8);
    d.step(false,b,1); CHECK(!d.compressorRunning()); CHECK(d.interruptedShortRuns()==1);
    d.step(true,b,5); CHECK(!d.compressorRunning());
    d.step(true,b,6); CHECK(d.compressorRunning());
    p.autoRestartAfterPowerRestore=false;
    DehumidifierModel standby(p); standby.step(true,b,100);
    CHECK(!standby.compressorRunning() && standby.feedback(0).feedback==DehumidifierFeedback::NoPowerDraw);
    p.autoRestartAfterPowerRestore=true; p.initialTankLevelLiters=4.999;
    DehumidifierModel tank(p); tank.step(true,b,100);
    CHECK(tank.tankFull() && !tank.compressorRunning());
    NEAR(tank.tankLevelLiters(),5,1e-10);
    CHECK(tank.feedback(0).feedback==DehumidifierFeedback::NoPowerDraw);
    tank.emptyTank(); CHECK(!tank.tankFull()); NEAR(tank.tankLevelLiters(),0,0);
    tank.step(true,b,100); CHECK(tank.compressorRunning());
    BasementModel cold({26,100,0},6,90), warm({26,100,0},20,90);
    DehumidifierModel slow, fast;
    slow.step(true,cold,1000); fast.step(true,warm,1000);
    CHECK(slow.effectiveExtractionLitersPerHour()<fast.effectiveExtractionLitersPerHour());
    p.performanceFactor=0; p.initialTankLevelLiters=0;
    DehumidifierModel broken(p); const double water=b.vaporMassGram();
    broken.step(true,b,1000); CHECK(broken.compressorRunning()); NEAR(b.vaporMassGram(),water,0);
    NEAR(broken.totalExtractedLiters(),0,0);
}); }
