# Changelog

A projekt változásai verziónként. A dátum formátuma ÉÉÉÉ-HH-NN.

## [0.1.0] - 2026-10-16

### Hozzáadva

- PHASE 1: C++17 projektstruktúra, CMake és PlatformIO native buildkonfiguráció.
- Platformfüggetlen ClimateReading és validált Configuration, 64 bites monoton időbélyeggel.
- Magnus-alapú telítési és aktuális vízgőznyomás, harmatpont és abszolút nedvesség.
- PHASE 2: közös VentilationController AUTO/ON/OFF módokkal, AH/RH hiszterézissel és minimum BE/KI időkkel.
- Azonnali automatikus leállás szenzorhiba, hideg pince vagy elégtelen kültéri nedvességelőny esetén.
- Emergency figyelmeztetés és későbbi páramentesítőhöz használható javaslatjelzés.
- ISensor, IRelay, IClock és IStorage interfészek.
- Két tesztprogram, összesen 88 ellenőrzéssel; Windows/MSVC alatt futtatva.
- Hardver-, biztonsági, architekturális és algoritmusdokumentáció, validálási jegyzőkönyv.
- GitHub Actions CMake/CTest ellenőrzés Linux és Windows rendszeren.

### Korlátok

- A GPIO-k és a relépolaritás fizikai ellenőrzést igényelnek.
- A PC dinamikus szimuláció, ESP32 adapterek, LVGL, NVS és REST a 3–8. fázisban készülnek.
- Ez a verzió még nem telepíthető hardvervezérlő firmware.
