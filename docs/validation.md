# Validálási jegyzőkönyv

## 0.2.0 — új specifikáció, Windows-szimulátor

Windows x64/MSVC Release: **11/11 CTest**, 8 C++ tesztprogramban **94 908 ellenőrzés**. Az új DryingController 10 000 randomállapotot és mindkét szenzor hibáit, a korábbi regresszió 20 000 állapotot futtat. Mind a 12 teljes napos forgatókönyv sikeres, vízmérlegzárás 1e-6 g tolerancián belül.

Fáziskapuk: a közös típusok/új konfiguráció és DryingController után 6/6 teszt; készülékmodell után 7/7; teljes futtató után 11/11. Minden következő fázis sikeres előző build/teszt után indult.

A Release EXE mind a 12 esetet lefuttatta és CSV/HTML kimenetet készített. Külön ellenőrizve: ékezetes kimeneti útvonal, CSV és beágyazott JSON egyezése, hibás CLI argumentumok nemnulla exit kódja. Dashboard JavaScript: 12 scenario, timeline és command/kompresszor különbség mock DOM tesztben sikeres. A helyi file:// oldalt a beépített böngésző biztonsági szabálya blokkolta, ezért valódi böngészős vizuális QA nem történt; nem kerültük meg a korlátozást.

A matplotlib tool Windows alatt PNG-t generált, a 8 paneles weather-change ábrát vizuálisan ellenőriztük. A dumpbin szerint a csomagolt EXE egyetlen közvetlen DLL függősége KERNEL32.dll; MSVC runtime statikusan linkelve. A későbbi hardveres boot, relé és NVS továbbra sincs hitelesítve.

A lenti 0.1.x adatok történeti jegyzőkönyvek.

0.1.1 kiegészítés: 4/4 tesztprogram, 20 393 sikeres ellenőrzés (40 + 48 + 20 004 + 301), 20 000 randomállapottal. Fizikai hibajavítás és a javítás előtti/utáni eredmények: [system_validation.md](system_validation.md). Az alábbiak az eredeti 0.1.0 fázisnapló adatai.

2026-10-16, Windows x64, CMake 3.31.6, Visual Studio 2022 / MSVC 19.44.35219.0.

PHASE 1: core könyvtár és test_psychrometrics build, CTest 1/1 sikeres, 48 ellenőrzés. Csak ezután indult PHASE 2.

PHASE 2: core könyvtár és két tesztprogram build, CTest 2/2 sikeres, összesen 88 ellenőrzés. A tesztmakrók Release buildben is aktívak, nem assert/NDEBUG függők.

Lefedett: ismert páraszámítási pontok; téli/nyári AH sorrend; hőmérsékleti határok; 0/100% RH; NaN/infinity/hibás RH; szenzorfrissesség pontos timeout-határa; jövőidejű adat; konfigurációvalidálás; indulás; normál leállás; minimum BE/KI időzítők; AH és RH hiszterézis; hideg és nedves kültér azonnali leállítása minimum BE alatt; szenzorhiba; kézi BE/KI/AUTO váltás; emergency jelzés; óra visszalépése.

Környezeti probléma: a szülőfolyamat duplikált Path/PATH változója kezdetben MSBuild hibát okozott; a Python buildrunner Windows alatt normalizálja a neveket. A tesztharness hiányzó string include-ja javítva; /EHsc explicit bekapcsolva. A sikeres végső buildben ezek nem maradtak fenn.

Nem hitelesített: PlatformIO build, ESP32 cross build, GPIO-k és fizikai reléműködés. NVS/reboot restore, PC dinamikus szimuláció és hét forgatókönyv, LVGL és REST a következő fázisok feladatai.
