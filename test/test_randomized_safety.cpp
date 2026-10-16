#include "core/VentilationController.h"
#include "core/Psychrometrics.h"
#include "Test.h"
#include <fstream>
#include <iomanip>
#include <random>
using namespace ventilation;
namespace p = ventilation::psychrometrics;
int main(int argc, char** argv) { return run([&] {
    std::mt19937 rng(0x5A17BEEFu);
    auto sample=[&](double lo, double hi) { return lo+(hi-lo)*static_cast<double>(rng())/4294967295.0; };
    std::ofstream csv;
    if (argc==2) {
        csv.open(argv[1]); CHECK(csv.good());
        csv << "case,basementT,basementRH,outdoorT,outdoorRH,rawDeltaAH,deltaVaporHpa,fanStart,fanRunning,outdoorDewPoint,surfaceT,hypotheticalSurfaceRisk\n";
        csv << std::setprecision(12);
    }
    int wetStarts=0, wetContinues=0, airCondensation=0, coldRuns=0, surfaceRisks=0, starts=0;
    constexpr int cases=20000;
    for (int i=0;i<cases;++i) {
        const double bt=sample(5,25), brh=sample(40,100), ot=sample(-15,38), orh=sample(20,100);
        Configuration cfg;
        if (i>=cases/2) { cfg.minHumidityAdvantage=0.1; cfg.stopHumidityAdvantage=0.01; }
        VentilationController start(cfg), running(cfg);
        ClimateReading b{bt,brh,0,true}, o{ot,orh,0,true};
        start.update(b,o,0);
        // Running path: previously dry winter weather, now abrupt weather change.
        running.update({20,90,0,true},{-15,20,0,true},0);
        running.update({20,90,300000,true},{-15,20,300000,true},300000);
        b.measuredAtMs=o.measuredAtMs=300001;
        const auto s=start.update(b,o,300001), r=running.update(b,o,300001);
        const double dv=p::vaporPressureHpa(bt,brh)-p::vaporPressureHpa(ot,orh);
        const double dp=*p::dewPointC(ot,orh);
        const bool wet=dv<=0;
        wetStarts += s.fanRequested && wet;
        wetContinues += r.fanRequested && wet;
        airCondensation += (s.fanRequested || r.fanRequested) && dp>=bt;
        coldRuns += (s.fanRequested || r.fanRequested) && bt<=cfg.minBasementTemperature;
        const double wall=bt-sample(0,8);
        const bool surfaceRisk=(s.fanRequested || r.fanRequested) && dp>=wall;
        surfaceRisks += surfaceRisk;
        starts += s.fanRequested;
        CHECK(std::isfinite(s.differenceGm3));
        if (csv) csv << i << ',' << bt << ',' << brh << ',' << ot << ',' << orh << ',' << s.differenceGm3
            << ',' << dv << ',' << s.fanRequested << ',' << r.fanRequested << ',' << dp << ',' << wall << ',' << surfaceRisk << '\n';
    }
    std::cout << "seed=0x5A17BEEF cases=" << cases << " wetStarts=" << wetStarts
        << " wetContinues=" << wetContinues << " airCondensation=" << airCondensation
        << " coldRuns=" << coldRuns << " starts=" << starts << " hypotheticalSurfaceRisks=" << surfaceRisks << '\n';
    CHECK(wetStarts==0); CHECK(wetContinues==0); CHECK(airCondensation==0); CHECK(coldRuns==0);
}); }
