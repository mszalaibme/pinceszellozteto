#include "simulator/SimulationRunner.h"
#include "Test.h"
using namespace ventilation;
using namespace ventilation::simulation;
int main() { return run([] {
    auto all=scenarios(); CHECK(all.size()==12);
    for(const auto& scenario:all) {
        const auto r=runScenario(scenario);
        bool vent=false,dehum=false,error=false,recovered=false,ineffective=false;
        bool prior=false; Timestamp offAt=0;
        for(const auto& row:r.rows) {
            CHECK(!(row.output.ventilationRequested && row.output.dehumidifierRequested));
            if(row.output.dehumidifierRequested && !prior) CHECK(row.timestamp-offAt>=600000);
            if(!row.output.dehumidifierRequested && prior) offAt=row.timestamp;
            prior=row.output.dehumidifierRequested;
            vent |= row.output.ventilationRequested; dehum |= row.output.dehumidifierRequested;
            error |= row.output.reasonCode==ReasonCode::NoPowerDraw;
            recovered |= row.timestamp>3600000 && row.output.dehumidifierRequested && row.compressorRunning;
            ineffective |= row.output.dehumidifierIneffective;
            if(scenario.id=="sensor-failure" && row.timestamp>=3600000)
                CHECK(!row.output.ventilationRequested && !row.output.dehumidifierRequested && row.output.state==ControllerState::SensorError);
        }
        NEAR(r.massBalanceErrorGram,0,1e-6);
        if(scenario.id=="winter") CHECK(vent && !dehum);
        if(scenario.id=="summer" || scenario.id=="emergency") CHECK(!vent && dehum && r.extractedLiters>0);
        if(scenario.id=="dry") CHECK(!vent && !dehum);
        if(scenario.id=="tank-full") CHECK(r.firstTankFullSeconds>0 && error);
        if(scenario.id=="empty-tank") CHECK(error && recovered);
        if(scenario.id=="no-restart") CHECK(error && r.compressorHours==0 && r.extractedLiters==0);
        if(scenario.id=="weather-change") CHECK(vent && dehum);
        if(scenario.id=="five-liters") { NEAR(r.generatedLiters,5,1e-8); CHECK(r.netVentilationLiters>4); }
        if(scenario.id=="ineffective") CHECK(ineffective && dehum && r.extractedLiters==0);
        std::cout<<scenario.id<<": vent="<<r.fanHours<<"h dehum="<<r.dehumCommandHours<<"h extracted="<<r.extractedLiters<<"L error="<<error<<'\n';
    }
}); }
