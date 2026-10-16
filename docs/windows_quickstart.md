# Windows szimulátor 0.5.0

A **START.cmd** vagy **pince-kijelzo.exe** megnyitja az interaktív kijelzőt. A képernyő gombjai kattinthatók; a jobb oldali próbapadon klíma, hibák, időléptetés és dinamika állítható. Az idő alapból áll: **+10 perc** után már letelnek az indulási minimum KI idők. Részletek: [kijelző útmutató](docs/display_simulator.md).

Kicsomagolás után a `simulation-output/analysis.html` azonnal megnyitható: 950 elkészült futás, hat összehasonlító grafikon és eredménytábla. A `report.html` hat részletes idősor visszajátszását és tíz grafikont tartalmazza. Telepítés és Python nem szükséges.

A `BATCH.cmd` újraszámolja a 950 futást a `config/baseline.ini` alapján, majd megnyitja az elemzést. Ez több másodpercet igényelhet. A konfigurációt szövegszerkesztővel módosíthatod.

```powershell
.\pinceszellozteto.exe --baseline --config config/baseline.ini --out baseline
.\pinceszellozteto.exe --batch --config config/baseline.ini --out simulation-output
.\pinceszellozteto.exe --calibrate config/calibration-observed-only.ini --out calibration
.\pinceszellozteto.exe --scenario all --out legacy-scenarios
```

`--physics-step 5` a fizikai integrálást, `--step 30` csak az exportot sűríti. A vezérlés üteme külön konfiguráció. A batch nedves összehasonlításai napi tartályürítést feltételeznek; a baseline nem.

A 0,21 L/h, 300 W és termikus adatok feltételezések. Az ismeretlen ventilátorfogyasztás nincs benne a részleges energiamutatóban. Az 5 L / 24 h megfigyelés önmagában nem kalibrálja a készüléket és a pincét. A vezérlés nagyobb nedvességforrásnál nem tartja az RH-célt. Részletek: `docs/simulation_addendum_results.md`.

A program Windows x64 natív szimulátor. ESP32 firmware, GPIO/230 V és készülék-autorestart hardvervalidáció még nem készült el.
