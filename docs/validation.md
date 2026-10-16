# Validálási jegyzőkönyv

0.1.1 kiegészítés: 4/4 tesztprogram, 20 393 sikeres ellenőrzés (40 + 48 + 20 004 + 301), 20 000 randomállapottal. Fizikai hibajavítás és a javítás előtti/utáni eredmények: [system_validation.md](system_validation.md). Az alábbiak az eredeti 0.1.0 fázisnapló adatai.

2026-10-16, Windows x64, CMake 3.31.6, Visual Studio 2022 / MSVC 19.44.35219.0.

PHASE 1: core könyvtár és test_psychrometrics build, CTest 1/1 sikeres, 48 ellenőrzés. Csak ezután indult PHASE 2.

PHASE 2: core könyvtár és két tesztprogram build, CTest 2/2 sikeres, összesen 88 ellenőrzés. A tesztmakrók Release buildben is aktívak, nem assert/NDEBUG függők.

Lefedett: ismert páraszámítási pontok; téli/nyári AH sorrend; hőmérsékleti határok; 0/100% RH; NaN/infinity/hibás RH; szenzorfrissesség pontos timeout-határa; jövőidejű adat; konfigurációvalidálás; indulás; normál leállás; minimum BE/KI időzítők; AH és RH hiszterézis; hideg és nedves kültér azonnali leállítása minimum BE alatt; szenzorhiba; kézi BE/KI/AUTO váltás; emergency jelzés; óra visszalépése.

Környezeti probléma: a szülőfolyamat duplikált Path/PATH változója kezdetben MSBuild hibát okozott; a Python buildrunner Windows alatt normalizálja a neveket. A tesztharness hiányzó string include-ja javítva; /EHsc explicit bekapcsolva. A sikeres végső buildben ezek nem maradtak fenn.

Nem hitelesített: PlatformIO build, ESP32 cross build, GPIO-k és fizikai reléműködés. NVS/reboot restore, PC dinamikus szimuláció és hét forgatókönyv, LVGL és REST a következő fázisok feladatai.
