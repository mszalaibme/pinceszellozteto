# ESP32 pince-szellőztetés

Buildelhető, platformfüggetlen C++17 vezérlőmag. **PHASE 1 és PHASE 2 kész és PC-n tesztelve.** A teljes termék a specifikáció szerinti további fázisokban készül; ez még nem telepíthető ventilátorvezérlő firmware.

Fejlesztési ág: [`dev`](https://github.com/mszalaibme/pinceszellozteto/tree/dev). Változások: [CHANGELOG.md](CHANGELOG.md), közreműködés: [CONTRIBUTING.md](CONTRIBUTING.md). A GitHub Actions a core buildjét és tesztjeit Windows és Linux környezetben ellenőrzi.

**0.1.1 rendszer-validáció:** 20 000 randomállapot, automatizált biztonsági regressziók és javított nedveslevegő-védelem. Eredmények és fennmaradó korlátok: [system_validation.md](docs/system_validation.md).

```sh
git clone --branch dev https://github.com/mszalaibme/pinceszellozteto.git
cd pinceszellozteto
```

## Fordítás és tesztek

Előfeltétel: CMake >=3.16 és C++17 fordító (MSVC, GCC vagy Clang).

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -V
```

Windows alatt Visual Studio C++ workload szükséges. A `tools/build_test.py` ugyanazt futtatja, és normalizálja a Windows környezeti változók neveit, ha a szülőfolyamat egyszerre adott át `Path` és `PATH` változót:

```sh
python tools/build_test.py --cmake "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
```

Opcionális PlatformIO native build: `pio run -e native` (host GCC kell). A PlatformIO build ebben a kiadásban nem volt futtatva; a hitelesített build CMake/MSVC.

## Architektúra

`core/`: tiszta standard C++, mérési adatok, képletek, konfiguráció, állapotgép. `hal/`: csak interfészek. `firmware/`: későbbi Arduino adapterek. `simulator/`: későbbi PC modell. `test/`: valódi core kód ellen futó önálló CTest programok. `src/`: PlatformIO native smoke belépőpont. `include/board_config.h`: ellenőrizetlen hardveradatok elkülönítése.

Az első feladatnak megfelelően a fázisok elkülönülnek:

| Fázis | Állapot |
|---|---|
| 1: skeleton, core típusok, pszichrometria, unit tesztek | Kész, 48 ellenőrzés |
| 2: állapotgép, időzítők, hiszterézis, unit tesztek | Kész, 40 ellenőrzés |
| 3: PC tömegmérlegmodell, hét forgatókönyv, CSV, plot | Következő fázis |
| 4: ESP32, SHT31, relé adapterek | Későbbi fázis |
| 5: LVGL kijelző, grafikon | Későbbi fázis |
| 6: NVS és konfiguráció-visszaállítás | Későbbi fázis |
| 7: Wi-Fi, web UI, REST | Későbbi fázis |
| 8: teljes rendszer integráció és dokumentáció | Későbbi fázis |

A konfiguráció most memóriában módosítható és validált; NVS, reboot-restore teszt, érintőszerkesztő és hálózat még nincs. A core invalid konstruktorparaméter esetén biztonságos alapkonfigurációra áll; futás közben invalid módosítást visszautasít. A mód induláskor AUTO, ventilátor KI.

**NEEDS HARDWARE VERIFICATION**: board revision, összes TFT/touch/SD/backlight/I2C pin, szabad GPIO, relépolaritás. Nincs kitalált GPIO és nincs engedélyezett hardverkimenet. Lásd [hardware.md](docs/hardware.md), [safety.md](docs/safety.md), [control_algorithm.md](docs/control_algorithm.md) és [validation.md](docs/validation.md).
