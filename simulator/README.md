# Windows szimulátor

A 0.2.0 már teljes futtatót tartalmaz: BasementModel + DehumidifierModel + SimulationRunner + DryingController. A 12 forgatókönyv definíciója a `scenarios()` függvényben van; ugyanazokat használja a CLI és a scenario teszt.

Indítás: [windows_quickstart.md](../docs/windows_quickstart.md). Modell: [simulation.md](../docs/simulation.md), [moisture_model.md](../docs/moisture_model.md), [dehumidifier.md](../docs/dehumidifier.md). A szenzorhiba csak a controllernek adott mérést érvényteleníti; a fizikai kültéri levegő ettől nem lesz NaN.
