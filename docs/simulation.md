# Windows PC szimuláció

Belépőpont: simulator/main.cpp → SimulationRunner → **DryingController**. A core nem függ a modelltől; firmware-ben ugyanezt a controllert kell a HAL-adapterekkel használni. Nem készült második, egyszerűsített szimulátoros vezérlési algoritmus.

A futtató a 12 forgatókönyv mindegyikén először érvényes méréseket és az előző fizikai intervallum végének feedbackjét adja a controllernek, majd a kérést a következő intervallumra alkalmazza. A CSV időbélyege másodperc, a core időbélyege 64 bites milliszekundum. Alap időlépés 60 s, CLI-ben 1…60 s. Egy exportminta a mérés időpontjának állapota és az azt követő intervallum parancsa; a tényleges kompresszorjel indulásnál késhet a konnektorhoz képest.

## Fizika

A pinceállapot vízgőztömeg [g], ebből AH, T-ből és AH-ból RH. Légcsere analitikus, hőmérséklet-különbségre korrigált bejövő vízgőzsűrűséggel. A falforrás g/h; a páramentesítő külön g vizet vesz ki; a telítési kondenzátum külön mérlegtag.

Intervallumonként a sorrend: légcsere+forrás → készülék vízkivonása → hőmérséklet. Ez operátorfelbontás; a kombinált modell nem minden időlépésre pontosan azonos, a kisebb lépés finomítja. Az állandó hőmérsékletű, készülék nélküli nedvesség-ODE analitikus megoldása továbbra is időlépésfüggetlen.

A termikus közelítés:

```
C * dT/dt = Hwall*(Twall-T) + 1200*Q/3600*(Tout-T) + Pelec
```

C=3 MJ/K, Hwall=35 W/K, Twall a forgatókönyv kezdeti pincehőmérséklete. Ezek szemléltető paraméterek. A konstans hővezetés/kapacitás nem mért falmodell, a légcsere névleges 100 m³/h. Csak elektromos hőterhelést számolunk, teljes nedveslevegő-entalpia és kondenzációs látens hő nélkül. Ez nem hitelesített hőegyensúly vagy kondenzációmentességi igazolás.

A készülékmodell kezeli a restartot, indulási késést, belső kompresszor KI időt, tényleges futást, teljesítményt és tartályt. Az empirikus kivonási görbe a dehumidifier.md fájlban szerepel.

## Forgatókönyvek

| ID | Cél |
|---|---|
| winter | 14°C/80%, kültér 5°C/90% → ventilátor |
| summer | 16°C/75%, kültér 28°C/65% → páramentesítő |
| dry | 15°C/58%, forrás 0 → minden KI |
| emergency | magas RH, nedvesebb kültér → páramentesítő |
| tank-full | 4.9 L kezdeti tartály, feltöltődés, command/feedback eltérés |
| empty-tank | ürítés az 1. órában, védett újraindulás és helyreálló diagnózis |
| no-restart | készülék standby; a szándékosan hibás capability-t a monitor leleplezi |
| compressor-protection | kétpercenként változó kültér, minimum KI ellenőrzése |
| sensor-failure | kültéri szenzor az 1. órától invalid, mindkét AUTO kimenet KI |
| weather-change | 24 órás nappal/éjszaka profil, stratégiaváltás |
| five-liters | 5000 g/nap nedvességforrás; ventilátor/készülék/tartály mérlege |
| ineffective | 310 W és futásfeedback, de nulla vízkivonás → warning |

A nagy falforrás meghaladhatja az aktuális szellőztetés szárítókapacitását: RH akkor is emelkedhet, ha a bejövő levegő szárazabb. A gyenge AH-előnynél tovább járó ventilátor és a 100% RH/kondenzátum a modellben lehetséges, nem rajzolási hiba. A jelenlegi vezérlő nem becsül falnedvességet és nem optimalizálja a két módszer várható g/h teljesítményét; ez későbbi szabályozásfejlesztési pont.

## Kimenet és futtatás

Lásd windows_quickstart.md. A standalone report.html nem tölt le könyvtárat, nem hív API-t és nem futtat szervert. Nyolc grafikon, forgatókönyvlista, időcsúszka és visszajátszás, állapotnapló és CSV-link található benne. A lejátszás előre kiszámolt adatot jelenít meg; a paraméterek újraszámítását az EXE végzi.

A Python-ábrázoló opcionális:

```powershell
python -m pip install -r tools/requirements-plot.txt
python tools/plot_simulation.py simulation-output/weather-change.csv --output weather.png
```

A CSV hiányzó szenzoradatot üres mezővel, a dashboard JSON null-lal jelöl; nem szerepel érvénytelen NaN JSON szám.
