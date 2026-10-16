# Changelog

A projekt változásai verziónként. A dátum formátuma ÉÉÉÉ-HH-NN.

## [0.1.2] - 2026-10-16

### Hozzáadva

- PC BasementModel vízgőztömeg állapotváltozóval; az AH és RH levezetett adatok.
- Analitikusan integrált légcsere és g/h nedvességforrás; a bejövő sűrűség pincehőmérsékletre számolva.
- Telítési átmenet és külön kondenzátum-mérleg; explicit előírt hőmérséklet, kitalált termikus modell nélkül.
- 30 034 új ellenőrzés, köztük 10 000 random modellállapot és zárt víztömegmérleg, valamint időlépésfüggetlenség.
- A meglévő 20 000 állapotos vezérlővalidáció változatlanul sikeres; összesen 50 427 helyi ellenőrzés.

## [0.1.1] - 2026-10-16

### Javítva

- AUTO nem indulhat és nem járhat tovább, ha a kültéri vízgőznyomás eléri a pincei értéket. Az eltérő hőmérsékletű levegők nyers g/m³ összehasonlítása önmagában hibás szárítási döntést engedett.
- A védelmi leállás a minimum BE időt is felülírja. Az AH kijelzés és konfigurált küszöbök megmaradnak.

### Validálás

- 20 000 determinisztikus randomállapot, induló és már járó ventilátorral, a megadott időjárási tartományokban.
- Javítás előtt 27 nedvesítő indulás és 51 nedvesítő továbbjárás; javítás után 0 és 0 ugyanazon mintákban.
- Regressziók: kétoldali szenzorhiba, NaN, infinity, timeout, jövőidejű minta, 32/64 bites időhatár, core újraindulás, relékapcsolási zaj és szélsőértékek.
- A fennmaradó fizikai és hardveres korlátok: [rendszervalidáció](docs/system_validation.md).

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
