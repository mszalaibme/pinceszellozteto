#include "core/Psychrometrics.h"
#include "core/ClimateData.h"
#include "core/Configuration.h"
#include "Test.h"
#include <limits>
using namespace ventilation;
using namespace ventilation::psychrometrics;
int main() { return run([] {
    NEAR(saturationVaporPressureHpa(0), 6.112, 0.000001);
    NEAR(saturationVaporPressureHpa(20), 23.326, 0.01);
    NEAR(vaporPressureHpa(20, 50), 11.663, 0.01);
    NEAR(absoluteHumidityGm3(20, 50), 8.622, 0.01);
    NEAR(*dewPointC(20, 50), 9.255, 0.01);
    NEAR(absoluteHumidityGm3(5, 90), 6.112, 0.03);
    CHECK(absoluteHumidityGm3(14, 80) > absoluteHumidityGm3(5, 90) + 1);
    CHECK(absoluteHumidityGm3(28, 65) > absoluteHumidityGm3(16, 75));
    for (double t : {-20., -10., 0., 20., 50.}) {
        NEAR(*dewPointC(t, 100), t, 1e-10);
        CHECK(saturationVaporPressureHpa(t) > 0);
        CHECK(*dewPointC(t, 50) < t);
        CHECK(calculate(t, 100).has_value());
    }
    CHECK(!dewPointC(20, 0));
    NEAR(absoluteHumidityGm3(20, 0), 0, 0);
    CHECK(calculate(20, 0).has_value());
    CHECK(!calculate(-21, 50)); CHECK(!calculate(51, 50));
    CHECK(!calculate(20, -1)); CHECK(!calculate(20, 101));
    CHECK(!calculate(NAN, 50)); CHECK(!calculate(20, INFINITY));
    CHECK(std::isnan(absoluteHumidityGm3(20, -1)));
    ClimateReading r{14, 80, 100, true};
    CHECK(r.isFresh(100, 120000)); CHECK(r.isFresh(120099, 120000));
    CHECK(!r.isFresh(120100, 120000)); CHECK(!r.isFresh(99, 120000));
    r.valid = false; CHECK(!r.isFresh(100, 120000));
    Configuration c; CHECK(c.isValid());
    c.stopVentilationHumidityAdvantage = 1; CHECK(!c.isValid());
    c = {}; c.emergencyRH = 60; CHECK(!c.isValid());
    c = {}; c.targetBasementRH = NAN; CHECK(!c.isValid());
    c = {}; c.sensorTimeout = 0; CHECK(!c.isValid());
}); }
