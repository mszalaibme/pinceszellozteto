KIEGÉSZÍTŐ RENDSZERSPECIFIKÁCIÓ A SZIMULÁCIÓHOZ

Az előző specifikációt egészítsd ki az alábbi konkrét adatokkal és
kalibrációs feltételekkel.

Ezeket használd alapértelmezett baseline modellként, de minden lényeges
paraméter maradjon konfigurálható.

==================================================
1. PINCE
==================================================

Pince alapterület:

12 m²

Becsült belmagasság:

2.2 m

Becsült légtérfogat:

26.4 m³

Használd alapértékként:

basementVolumeM3 = 26.4


A pince régi, nedves épületszerkezetű pince.

Jelentős, folyamatos nedvességterhelés érkezhet:

- falakból
- talaj felől
- szerkezetek párolgásából

A moisture source ezért NE legyen konstansként beégetve.

Legyen:

moistureIngressGPerHour

konfigurálható paraméter.

A szimuláció kalibrációja során ezt vissza lehet becsülni a valós
mérésekből.

==================================================
2. PINCEI ELSZÍVÓ
==================================================

Meglévő csőventilátor:

nominalAirflowM3h = 100.0

230 V AC

Elszívó ventilátor.

A pince nincs közös légtérben a lakóházzal.

A ventilátor a pince levegőjét közvetlenül kültérre fújja ki.

Pótlólevegő kültérről érkezik.

A tényleges légszállítás ismeretlen, ezért a szimulátorban legyen:

ventilationAirflowM3h = 100

de legyen konfigurálható.

Készíts sensitivity simulationt legalább:

60 m³/h
80 m³/h
100 m³/h
120 m³/h

értékekkel.

A 26.4 m³ térfogattal a névleges 100 m³/h:

ACH = 100 / 26.4
≈ 3.79 1/h

==================================================
3. PÁRAMENTESÍTŐ
==================================================

Meglévő készülék:

- 230 V AC
- kompresszoros páramentesítő
- beépített kb. 5 literes víztartály

A pontos gyári modell / rated extraction / fogyasztás jelenleg NEM ISMERT.

NE találj ki gyári specifikációt.

Jelöld:

NEEDS HARDWARE VERIFICATION


Viszont van valós megfigyelési adat:

Nagyon nedves téli időszakban az 5 literes tartály
néha kb. 24 óra alatt megtelik.

Ez azt jelenti, hogy megfelelő körülmények között a készülék
tényleges átlagos vízkivonása legalább:

5 L / 24 h
= 0.2083 L/h
≈ 208 g/h

lehet.

Ezt használjuk EMPIRIKUS KALIBRÁCIÓS PONTKÉNT.

FONTOS:

Ez NEM a készülék rated extraction értéke.

Ez egy valós, adott pinceállapot mellett megfigyelt effektív
vízkivonási érték.

==================================================
4. DEHUMIDIFIER MODELL BASELINE
==================================================

Amíg a pontos típus nem ismert, a szimulációhoz használj egy
konfigurálható empirikus modellt.

Baseline:

referenceExtractionLh = 0.21

Ez kb. 5 L / nap.

De a tényleges extraction függjön:

- basement temperature
- basement RH

értékektől.

Ne legyen konstans 0.21 L/h minden körülmény között.

Javasolt első közelítés:

effectiveExtraction =
referenceExtraction
* temperatureFactor(T)
* humidityFactor(RH)


Referencia környezet például:

referenceTemperatureC = 15
referenceRH = 80


Ezen a ponton:

temperatureFactor = 1
humidityFactor = 1

tehát:

effectiveExtraction ≈ 0.21 L/h


Az alacsonyabb RH csökkentse jelentősen a vízkivonást.

Az alacsonyabb hőmérséklet szintén csökkentse a kompresszoros
páramentesítő hatásosságát.

A függvény legyen dokumentált és könnyen cserélhető később a készülék
valós performance curve-jére.

==================================================
5. PÁRAMENTESÍTŐ TESZT-TARTOMÁNY
==================================================

Mivel a tényleges effektív teljesítmény még bizonytalan, futtass
sensitivity simulationt legalább ezekkel a reference extraction
értékekkel:

0.10 L/h
0.15 L/h
0.21 L/h
0.30 L/h
0.40 L/h

Ez rendre:

2.4 L/day
3.6 L/day
5.0 L/day
7.2 L/day
9.6 L/day

körülbelüli referencia vízkivonást jelent.

Így a szimuláció ne függjön kritikusan egy még nem ismert készülékadattól.

==================================================
6. TARTÁLY
==================================================

tankCapacityLiters = 5.0

A tartály szimulációja legyen valódi vízmérleg:

tankLevel += extractedWater

Ha:

tankLevel >= 5.0

akkor:

tankFull = true

compressorRunning = false

A vezérlő oldali:

dehumidifierRequested

ettől még lehet true.

Ezzel szimuláljuk a valós esetet:

a konnektor kap áramot,
de a készülék tele tartály miatt már nem páramentesít.

==================================================
7. PÁRAMENTESÍTŐ TELJESÍTMÉNYFELVÉTEL
==================================================

A pontos elektromos fogyasztás jelenleg nem ismert.

NE találj ki konkrét wattértéket valós hardver-specifikációként.

A szimulációhoz használj konfigurálható baseline értéket:

dehumidifierPowerW = 300

DE ezt egyértelműen jelöld:

SIMULATION ASSUMPTION

és futtass energia sensitivity tesztet:

200 W
250 W
300 W
400 W
500 W

értékekkel.

A későbbi valós adat egyszerűen felülírhassa ezt.

==================================================
8. KOMPRESSZOR
==================================================

A kompresszor védelmére baseline:

minimumOnTime = 15 min

minimumOffTime = 10 min

startupGracePeriod = 2 min

Ezek konfigurálhatók.

A kompresszor minimum OFF idejét minden automatikus újraindításnál
tartsd be.

==================================================
9. SZELLŐZTETÉS VS PÁRAMENTESÍTŐ
==================================================

A döntés alapja elsősorban az abszolút nedvesség legyen.

Definíció:

deltaAH =
basementAbsoluteHumidity
-
outsideAbsoluteHumidity


Baseline:

ventilationStartDeltaAH = 1.0 g/m³

ventilationStopDeltaAH = 0.4 g/m³


Ha:

basementRH > targetRH

és

deltaAH >= 1.0 g/m³

és

basementTemp > minBasementTemperature

akkor:

VENTILATE


Ha:

basementRH > targetRH

de

deltaAH < 1.0 g/m³

akkor:

DEHUMIDIFY


Ha:

basementRH <= targetRH

akkor:

IDLE


Baseline:

targetRH = 65 %

startRHHysteresis = 3 %

minBasementTemperature = 10 °C

==================================================
10. FONTOS: SZELLŐZTETÉS HŐHATÁSA
==================================================

A ventilációs szimuláció NE csak nedvességet cseréljen.

A kültéri levegő a pincébe érkezik, ezért a pince levegőjének
hőmérsékletére is hat.

Készíts egyszerű termikus modellt.

A pince ne azonnal vegye fel a kültéri hőmérsékletet.

Legyen benne:

- air thermal mass
- effective building thermal mass
- heat exchange with surrounding soil/walls
- outdoor ventilation heat exchange

A fal/talaj nagy termikus tömege miatt a pince hőmérséklete lassabban
változzon, mint a levegő önmagában.

Használj egy konfigurálható egyszerű lumped thermal modelt.

Például:

effectiveThermalCapacityJPerK

wallTemperatureC

wallAirHeatTransferWPerK


Ne állíts konkrét szerkezeti értéket tényként.

Használj dokumentált baseline értékeket és sensitivity analysis-t.

==================================================
11. PÁRAMENTESÍTŐ HŐHATÁSA
==================================================

A kompresszoros páramentesítő:

- vizet von ki
- elektromos energiát használ
- hőt ad le a pincébe

A szimulációban a páramentesítő által felvett villamos energia
közel teljes egésze végül hőként jelenik meg a pince légterében /
szerkezeteiben.

Ezért amikor a páramentesítő működik:

add approximately

dehumidifierPowerW

thermal power to the basement thermal model.

A kondenzáció látens hőjét is kezeld, ha a modell részletessége
indokolja.

Dokumentáld az egyszerűsítést.

==================================================
12. VALÓS MEGFIGYELÉSI ADAT
==================================================

A rendszer kalibrációjához használd fel ezt a valós tapasztalatot:

Nagyon nedves időszakban:

5 literes tartály
≈ 1 nap alatt megtelik.

Egy megfelelően beállított szimuláció egyik célja az legyen,
hogy legyen olyan reális pince-nedvességterhelési paraméterezés,
amelynél ezt reprodukálni tudjuk.

NE erőltesd minden időjárási helyzetben az 5 L/day értéket.

Ez csak kalibrációs pont.

==================================================
13. KALIBRÁCIÓS MÓD
==================================================

Készíts külön simulation calibration módot.

Bemenetek például:

measuredBasementTemperature
measuredBasementRH

measuredOutdoorTemperature
measuredOutdoorRH

measuredWaterCollectedLiters

measurementDurationHours

fanRuntimeHours

dehumidifierRuntimeHours


A calibration tool becsülhesse:

moistureIngressGPerHour

és/vagy

effectiveDehumidifierExtractionLh


Később valós mért adatokkal tudjuk finomítani a modellt.

==================================================
14. SZIMULÁCIÓS OUTPUT BŐVÍTÉS
==================================================

CSV-ben szerepeljen:

timestamp

basementTempC
basementRH
basementAH
basementDewPoint

outsideTempC
outsideRH
outsideAH
outsideDewPoint

deltaAH

moistureIngressGPerHour

ventilationAirflowM3h

ventilationWaterRemovalGPerHour

dehumidifierWaterRemovalGPerHour

dehumidifierPowerW

tankLevelLiters

tankFull

ventilationCommand

dehumidifierCommand

controllerState

cumulativeVentilationWaterRemovedL

cumulativeDehumidifierWaterRemovedL

cumulativeEnergyKWh

==================================================
15. FONTOS MUTATÓK A SZIMULÁCIÓ VÉGÉN
==================================================

Minden simulation scenario végén készíts összefoglalót:

Average basement RH

Maximum basement RH

Minimum basement temperature

Ventilation runtime hours

Dehumidifier runtime hours

Water removed by ventilation [L]

Water removed by dehumidifier [L]

Number of tank-full events

Estimated electricity consumption [kWh]

Time above 70 % RH

Time above 80 % RH

Time below minimum basement temperature

==================================================
16. BASELINE SZCENÁRIÓ
==================================================

Készíts egy baseline 7 napos szimulációt.

Basement initial state:

T = 14 °C
RH = 80 %

Tank:

0 L

Ventilation:

100 m³/h

Volume:

26.4 m³

Controller:

targetRH = 65 %
startDeltaAH = 1.0 g/m³
stopDeltaAH = 0.4 g/m³
minimumBasementTemperature = 10 °C


A kültéri időjárás 24 órás profilt kövessen:

éjjel:

T ≈ 4-7 °C
RH ≈ 85-95 %

nappal:

T ≈ 8-12 °C
RH ≈ 65-85 %


Használj fokozatos változást,
ne hirtelen négyszögjelet.

A moisture ingress értéket sweepeld több értéken.

Például:

50 g/h
100 g/h
150 g/h
200 g/h
250 g/h

és mutasd meg, hogyan változik a rendszer működése.

==================================================
17. PARAMÉTER-SWEEP
==================================================

Készíts automatikus batch simulation toolt.

Sweepeld legalább:

ventilation airflow:
60 / 80 / 100 / 120 m³/h

dehumidifier reference extraction:
0.10 / 0.15 / 0.21 / 0.30 / 0.40 L/h

moisture ingress:
50 / 100 / 150 / 200 / 250 g/h

target RH:
60 / 65 / 70 %

minimum basement temperature:
8 / 10 / 12 °C


Az eredményt CSV summary táblába exportáld.

A cél, hogy lássuk:

milyen paraméterek mellett működik jól a szabályozás,
és melyekre érzékeny a rendszer.

==================================================
18. FONTOS FEJLESZTÉSI SZABÁLY
==================================================

A jelenleg ismeretlen hardveradatokat SOHA ne cseréld le
hallucinált gyári specifikációkra.

Használj:

SIMULATION ASSUMPTION

vagy:

NEEDS HARDWARE VERIFICATION

jelölést.

A szimuláció minden ilyen paramétert konfigurációból kapjon.

Így amikor meglesz:

- páramentesítő pontos típusa
- valódi wattfogyasztása
- névleges L/day értéke
- fan tényleges légszállítása
- valós pince mérési adatok

azokat egyszerűen be tudjuk írni, anélkül hogy a modellt át kellene
tervezni.

==================================================
19. MOSTANI FELADAT
==================================================

Az előzőleg elkészült projektet NE kezdd újra.

Integráld ezt a specifikációt a meglévő modellbe.

Ezután:

1. ellenőrizd, mely paraméterek voltak eddig hardcode-olva
2. tedd őket Configuration-be
3. implementáld a hiányzó thermal modelt
4. implementáld a calibration támogatást
5. implementáld a parameter sweepet
6. futtasd a baseline 7 napos szimulációt
7. futtasd a sensitivity teszteket
8. generálj CSV outputot
9. generálj grafikonokat
10. írj rövid eredmény-összefoglalót

Ha valamelyik eredmény fizikailag gyanús, ne csak fogadd el:
vizsgáld meg a modellt és a mértékegységeket.