# Pince-szellőztető és páramentesítő vezérlő

**0.5.0 · Windows x64 szimulátor + platformfüggetlen C++17 vezérlőmag.**

A frissített [alapspecifikáció](docs/specification-v2.hu.md) szerint a rendszer szellőztetés, páramentesítés és IDLE között választ. Az új `DryingController` azonos kódja fut a PC-s szimulátorban és a tesztekben; a későbbi firmware ezt használja majd. A pince modellje vízgőztömeget számol, az RH levezetett érték.

A [szimulációs kiegészítés eredményei](docs/simulation_addendum_results.md) tartalmazzák a 900 kombinációs rácsot, a kalibráció korlátait és a feltárt szabályozási problémát. A 0,21 L/h és 300 W feltételezés, nem gyári adat.

## Interaktív kijelző Windowsra

A főoldalon a készülékek állapota nagy, közérthető feliratként jelenik meg. Az indítás előtti védelmi szünetnél a készülék neve és a hátralévő perc/másodperc látszik. A delta, abszolút nedvesség és harmatpont a KÉSZÜLÉK → MŰSZAKI ADATOK oldalra került. A „MŰKÖDIK” csak visszajelzéssel igazolt páramentesítőnél jelenik meg.

A **START.cmd** a **pince-kijelzo.exe** alkalmazást indítja. Kattintható 320×480-as kijelző, négy oldal, AUTO/VENTI/PÁRAM./OFF, 18 beállítás, nyolc előre beállított próba, szerkeszthető klíma, hibainjektálás, időgyorsítás, reboot és tartályürítés. A döntést a meglévő C++ DryingController hozza; a Windows-rajzolás natív GDI UI-prototípus. A firmware LVGL-renderelője még nincs implementálva.

[Kijelző használati útmutató](docs/display_simulator.md). A rögzített klíma a döntések próbája; bekapcsolható a víztömeg- és hőmodell is. Az idő alapból áll, a minimum KI idők érvényesek.

## Windows batch szimuláció

A GitHub Actions sikeres futásából letölthető a **pinceszellozteto-windows-x64** artifact. Kicsomagolás után **BATCH.cmd** futtatása indítja a 950 paraméteres szimulációt és megnyitja az offline grafikus riportot. Az artifact az EXE-t, előállított CSV-ket, a riportot és az indítási útmutatót is tartalmazza. Az aktuális feladat átadási ZIP-je ugyanezt a hordozható csomagot adja.

Nincs szükség ESP32-re, LVGL-re, 230 V-ra, Pythonra vagy telepítésre. A böngészőben forgatókönyv-választó, időcsúszka, visszajátszás, tíz grafikon és állapotnézet érhető el. A böngésző már kiszámolt adatokat mutat; új paraméterekkel az EXE számol újra.

```powershell
.\pinceszellozteto.exe --batch --config config/baseline.ini --out simulation-output
.\pinceszellozteto.exe --baseline --config config/baseline.ini --out baseline
.\pinceszellozteto.exe --calibrate config/calibration-observed-only.ini --out calibration
.\pinceszellozteto.exe --list
.\pinceszellozteto.exe --scenario all --hours 24 --step 60 --out simulation-output
.\pinceszellozteto.exe --scenario winter --source 208.333333 --out terheles
```

[Windows gyorsindító](docs/windows_quickstart.md) · [Szimuláció és forgatókönyvek](docs/simulation.md) · [Vezérlés](docs/control_algorithm.md) · [Páramentesítő](docs/dehumidifier.md)

## Mi készült el?

- AUTO / MANUAL_OFF / MANUAL_VENTILATION / MANUAL_DEHUMIDIFIER; AUTO-ban kizárt az egyidejű működés.
- Cél+RH-hiszterézis indítás, cél-RH leállási igény, AH hiszterézis, tényleges szárítási irányt ellenőrző vízgőznyomás-védelem.
- Külön minimum BE/KI idők; kompresszor-késleltetés boot és manuális kérés után is.
- Szenzorhibánál mindkét AUTO kimenet KI; kézi kimenet szenzorfigyelmeztetéssel lehetséges.
- Restart capability, command/feedback megkülönböztetés, no-power grace, készülékhiba, hatástalansági warning, opcionális ESTIMATED tartály.
- Víztömegmérleg, falforrás, analitikus légcsere, egyszerű termikus modell, T/RH-függő készülékkivonás, tényleges tartálytelítettség és ürítés.
- Mind a 12 előírt forgatókönyv, CSV és offline HTML grafikonok; opcionális matplotlib tool.
- Automatizált Windows Release build és tesztek, Unicode fájlút és dashboard-logika ellenőrzés.

A régi `VentilationController` a történeti regressziókhoz megmaradt. Új integrációhoz kizárólag `DryingController` használandó. A konfigurációs mezők új nevei az aktuális specifikációt követik; lásd [tervezési döntések](docs/design_decisions_v2.md).

## Készültség az új fázisok szerint

| Fázis | Állapot |
|---|---|
| 1: típusok, pszichrometria, konfiguráció, tesztek | Kész |
| 2: DryingController és két fogyasztó kiválasztása | Kész, tesztelt |
| 3: készülék-, tartály-, feedbackmodell, kompresszorvédelem | Kész, tesztelt |
| 4: teljes PC-futtató, 12 forgatókönyv, CSV, grafikon | Kész, Windows-csomag |
| 5: ESP32 szenzor/relé/monitor adapterek | Interfészek kész; hardverimplementáció hátra |
| 6: LVGL kijelző | Windows UI-prototípus és interakciók kész; firmware LVGL-renderelő hátra |
| 7: NVS és settings | Hátra |
| 8: Wi-Fi, REST, firmware web UI | Hátra |
| 9: teljes hardveres validáció | Hátra; core-regressziók már futnak |

Az offline szimulációs HTML nem a firmware REST/web UI-ja. A PC idősortárolás nem helyettesíti a későbbi 24 órás firmware RAM ring buffert. Valós NVS/config restore, boot GPIO és 230 V üzem nincs kész vagy hitelesítve.

## Forrásból fordítás

Windows 10/11 x64, Visual Studio 2022 C++ workload és CMake >=3.16:

```powershell
git clone --branch dev https://github.com/mszalaibme/pinceszellozteto.git
cd pinceszellozteto
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure -V
.\build\Release\pinceszellozteto.exe --scenario all --out simulation-output
```

A `tools/build_test.py --config Release --cmake ABSZOLUT_CMAKE_UT` ugyanígy fordít és tesztel, Windows Path/PATH környezeti ütközést is kezel. A PlatformIO native környezet smoke buildhez áll rendelkezésre, nem ellenőrizetlen ESP32 board targetként.

## Dokumentáció és biztonság

[Architektúra](docs/architecture.md) · [Tömegmérleg](docs/moisture_model.md) · [Hardver](docs/hardware.md) · [Biztonság](docs/safety.md) · [Validálás](docs/validation.md) · [CHANGELOG](CHANGELOG.md)

**NEEDS HARDWARE VERIFICATION:** board revision, TFT/touch/SD/backlight/I2C, szabad GPIO, mindkét relépolaritás, monitor. **NEEDS ELECTRICAL DESIGN / VERIFICATION:** kompresszorhoz alkalmas hálózati kapcsolás/kontaktor. A restart capability alapból false; csak a konkrét készüléken ellenőrzött képesség engedélyezhető.

A modellek nem garantálják a valódi pince felületeinek kondenzációmentességét vagy a két mérés közötti túlhűlés kizárását. [Korábbi rendszer-validáció és korlátok](docs/system_validation.md).

## Beszerzési előkészítés

[HESTORE alkatrészlista és nyitott hardverkérdések, 2026-10-16](docs/hestore_bom_2026-10-16.md): 24 kosártétel, a megadott érzékelő- és készüléktávolságokkal, beszerzési idő és telepítési feltételek. Előzetes kosár, nem teljes telepítési terv.

