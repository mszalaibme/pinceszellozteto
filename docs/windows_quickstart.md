# Pince szimulátor — Windows x64, 0.2.0

## Leggyorsabb indítás

1. Csomagold ki a ZIP teljes tartalmát egy írható mappába.
2. Kattints a **START.cmd** fájlra. Ez lefuttatja a 12 forgatókönyvet és megnyitja az eredményeket az alapértelmezett böngészőben.
3. A forgatókönyvet a felső listában, az időpontot a csúszkával választhatod. A lejátszógomb a már kiszámított idősort játssza vissza.

Az eredmények előre is szerepelnek a csomagban: `simulation-output/report.html` közvetlenül megnyitható, újraszámítás nélkül. A böngésző csak offline megjelenítő; nem ESP32 webes felület és nem szerver. A futtatáshoz nincs szükség Pythonra, PlatformIO-ra, Visual Studióra, internetre vagy hardverre. Az EXE statikusan linkelt MSVC runtime-ot használ; külön VC++ runtime telepítése nem szükséges. A minimum célrendszer Windows 10/11 x64. A helyi build és futtatás Windows 10 x64 alatt tesztelve.

## Egyedi futtatás PowerShellből

```powershell
.\pinceszellozteto.exe --list
.\pinceszellozteto.exe --scenario summer --hours 48 --step 60 --out eredmenyek
.\pinceszellozteto.exe --scenario winter --source 208.333333 --hours 24 --out terheles
.\pinceszellozteto.exe --scenario weather-change --hours 24 --step 10 --out idojaras
```

A `--source` a falakból érkező nedvesség grammban/órában. A `--step` másodpercben 1…60, az időtartam legfeljebb 168 óra, összesen legfeljebb 250 000 exportminta. Minden futtatás saját CSV-ket és önálló `report.html` fájlt ír az adott célmappába. Azonos célmappában az azonos nevű eredményeket felülírja. Az ékezetes kimeneti mappaneveket külön teszt ellenőrzi.

## Mit mutat?

- Pincei és kültéri hőmérséklet, RH, AH, harmatpont, ΔAH.
- Ventilátorkérés, konnektorkérés, valódi modellkompresszor, feedback és teljesítmény.
- A modelltartály tényleges vízszintje és külön ESTIMATED becslés.
- Összes üzemidő, tartályba kivont víz, légcserével nettó elvitt víz, mérlegeltérés.
- Szenzorhiba, tele tartály/nincs működés, automatikus restart hiánya és hatástalan működés.

A tartály becslése nem biztonsági érzékelő. A konnektor BE nem jelenti azt, hogy jár a kompresszor. A modellek paraméterezett közelítések, nem a konkrét pince és készülék hitelesített digitális másai.

Az ESP32 kijelző/LVGL és firmware még nincs kész ebben a kiadásban. A GPIO-k és a hálózati rész ellenőrzésre várnak.
