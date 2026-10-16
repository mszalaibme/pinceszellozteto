Egy komplett, jól strukturált ESP32 alapú pince-szellőztető és páramentesítő vezérlőrendszert szeretnék létrehozni.

Ne csak mintakódot írj.
Hozd létre a teljes projektstruktúrát, build konfigurációt, platformfüggetlen vezérlési logikát, ESP32 firmware-t, kijelző UI-t, teszteket, PC-n futó szimulátort és dokumentációt.

A projektet úgy tervezzük meg, hogy a vezérlési algoritmus hardver nélkül is teljes egészében szimulálható és automatikusan tesztelhető legyen.

==================================================
1. A RENDSZER CÉLJA
==================================================

Egy nedves, kb. 12 m²-es pincét szeretnék automatikusan szárítani.

A pince:
- kb. 12 m²
- kb. 2,2 m belmagasság
- térfogata kb. 26 m³
- nincs közös légtere a lakóházzal
- jelentős fal/talaj eredetű nedvességterhelése van

Jelenleg rendelkezésre áll:

1. kb. 100 m³/h teljesítményű 230 V-os csőventilátor
   - kifelé szívja a pince levegőjét
   - kültéri levegő szolgál pótlólevegőként

2. 230 V-os kompresszoros páramentesítő
   - kb. 5 literes víztartállyal
   - nedves téli időszakban akár kb. 5 liter vizet is összegyűjthet 1 nap alatt
   - megtelt tartály esetén a készülék saját biztonsági rendszere leállítja a páramentesítést

A cél az, hogy a rendszer mindig a megfelelő szárítási módot válassza:

- ha a kültéri levegő alkalmas a pince szárítására:
  SZELLŐZTETÉS

- ha a kültéri levegő nem alkalmas szárításra:
  PÁRAMENTESÍTŐ

- ha nincs szükség szárításra:
  MINDEN OFF

A rendszer NE kizárólag relatív páratartalom alapján döntsön.

Figyelembe kell vennie:
- hőmérséklet
- relatív páratartalom
- harmatpont
- abszolút nedvességtartalom [g/m³]

==================================================
2. TERVEZETT HARDVER
==================================================

Fő vezérlő:

ESP32-3248S035C-CYD
- ESP32
- 3,5" 320x480 kijelző
- kapacitív touch
- Wi-Fi

Szenzorok:

Pince:
- SHT30 vagy SHT31
- I2C

Kültér:
- DFRobot SEN0385 / SHT31 vagy kompatibilis kültéri SHT31
- I2C

Kiegészítők:

- MCP23017 I/O expander, ha szükséges
- relé a ventilátor vezérlésére
- külön megfelelő teljesítményű relé/kontaktor a páramentesítő konnektorának vezérlésére

FONTOS:

A páramentesítő kompresszoros terhelés.

NE feltételezd, hogy egy kis ESP32 relémodul alkalmas a készülék közvetlen kapcsolására.

Az architektúra úgy készüljön, hogy:

ESP32
  -> kisfeszültségű vezérlőkimenet
  -> megfelelő relé / kontaktor
  -> vezérelt 230 V-os konnektor
  -> páramentesítő

A hálózati feszültségű rész fizikailag és dokumentációban is különüljön el a kisfeszültségű elektronikától.

A konkrét hálózati kapcsolást ne feltételezd és ne improvizáld.

Jelöld:

NEEDS ELECTRICAL DESIGN / VERIFICATION

ahol szükséges.

==================================================
3. FONTOS HARDVER-ELLENŐRZÉSEK
==================================================

A projekt dokumentációjában külön szerepeljen:

1.
Meg kell vizsgálni, hogy a páramentesítő áramszünet / tápelvétel után automatikusan újraindul-e.

Ha visszakapcsolt táp után csak standby állapotba kerül és fizikai POWER gombot igényel, a kapcsolt konnektor önmagában nem elegendő.

Legyen konfigurálható capability:

dehumidifierAutoRestartSupported

2.
A páramentesítő tényleges működésének monitorozása külön interfészen történjen.

Ne feltételezd, hogy attól, hogy a vezérelt konnektor ON, a páramentesítő ténylegesen működik.

Tervezd be:

IDehumidifierMonitor

interfészt.

Lehetséges későbbi implementációk:

- áramfelvétel mérés
- teljesítménymérés
- intelligens konnektor visszajelzés
- dedikált állapotkontakt
- egyéb hardveres feedback

Első hardververzióban lehet:

Unknown / NotAvailable

állapot is.

==================================================
4. PÁRAMENTESÍTŐ TARTÁLYÁNAK KEZELÉSE
==================================================

A páramentesítő tartálya kb. 5 literes.

A rendszer kezelje külön állapotként:

DEHUMIDIFIER_TANK_FULL

vagy általánosabban:

DEHUMIDIFIER_NOT_RUNNING

Ha a vezérlő bekapcsolta a páramentesítőt, de a monitor szerint a gép nem fogyaszt / nem dolgozik, akkor megfelelő idő után generáljon hibát.

Például:

Commanded ON
+
measuredPower < configured minimum
+
startupGracePeriod lejárt
=
DEHUMIDIFIER_NOT_RUNNING

A kijelzőn jelenjen meg például:

"Páramentesítő nem működik"
"Ellenőrizd a víztartályt!"

A rendszer nem feltétlenül tudja biztosan megkülönböztetni:

- tele tartály
- belső készülékhiba
- standby állapot
- power-loss restart hiánya

Ezért a belső diagnosztikai állapot legyen pontos:

DEHUMIDIFIER_NO_POWER_DRAW

és a felhasználói UI mondhatja:

"Ellenőrizd a víztartályt / páramentesítőt"

==================================================
5. SZOFTVER STACK
==================================================

Használj:

- PlatformIO
- Arduino framework ESP32-höz
- modern C++
- LVGL
- Preferences / NVS
- ArduinoJson, ha szükséges

A core üzleti logika SEMMILYEN ESP32-specifikus függőséget ne tartalmazzon.

A vezérlési algoritmus fusson ugyanúgy:

- ESP32 firmware-ben
- PC-s szimulátorban
- unit tesztekben

==================================================
6. JAVASOLT PROJEKTSTRUKTÚRA
==================================================

/
├── platformio.ini
├── README.md
│
├── docs/
│   ├── architecture.md
│   ├── hardware.md
│   ├── control_algorithm.md
│   ├── dehumidifier.md
│   ├── simulation.md
│   └── safety.md
│
├── core/
│   ├── ClimateData.*
│   ├── Psychrometrics.*
│   ├── DryingController.*
│   ├── ControllerState.*
│   ├── Configuration.*
│   ├── DehumidifierState.*
│   └── Diagnostics.*
│
├── hal/
│   ├── ISensor.*
│   ├── IRelay.*
│   ├── IClock.*
│   ├── IStorage.*
│   └── IDehumidifierMonitor.*
│
├── firmware/
│   ├── sensors/
│   ├── display/
│   ├── relay/
│   ├── dehumidifier/
│   ├── storage/
│   └── network/
│
├── simulator/
│   ├── main.cpp
│   ├── BasementModel.*
│   ├── OutdoorModel.*
│   ├── DehumidifierModel.*
│   ├── VentilationModel.*
│   └── scenarios/
│
├── test/
│   ├── test_psychrometrics.cpp
│   ├── test_controller.cpp
│   ├── test_dehumidifier.cpp
│   └── test_scenarios.cpp
│
└── tools/
    └── plot_simulation.py

Ha ennél jobb struktúrát javasolsz, módosíthatod, de dokumentáld az indoklást.

==================================================
7. PSZICHROMETRIA
==================================================

Implementálj jól tesztelt számításokat:

- telítési vízgőznyomás
- aktuális vízgőznyomás
- harmatpont [°C]
- abszolút nedvességtartalom [g/m³]

Input:

T [°C]
RH [%]

Output:

dewPointC
absoluteHumidityGm3

Működési tartomány kb.:

-20 ... +50 °C

Dokumentáld a képleteket és írj unit teszteket.

==================================================
8. ÜZEMMÓDOK
==================================================

A rendszernek legyen:

AUTO
MANUAL_OFF
MANUAL_VENTILATION
MANUAL_DEHUMIDIFIER

Opcionálisan:

MANUAL_BOTH

de automatikus üzemben a BOTH alapértelmezésben legyen tiltva.

==================================================
9. AUTOMATIKUS VEZÉRLÉS
==================================================

Alap konfiguráció:

targetBasementRH = 65 %

rhStartHysteresis = 3 %

emergencyRH = 80 %

minVentilationHumidityAdvantage = 1.0 g/m³

stopVentilationHumidityAdvantage = 0.4 g/m³

minBasementTemperature = 10 °C

ventilationMinimumOnTime = 10 perc

ventilationMinimumOffTime = 5 perc

dehumidifierMinimumOnTime = 15 perc

dehumidifierMinimumOffTime = 10 perc

dehumidifierStartupGracePeriod = 2 perc

sensorTimeout = 120 másodperc

Minden érték Configuration objektumban legyen.

Semmi ne legyen hardcode-olva.

==================================================
10. VENTILÁCIÓS DÖNTÉS
==================================================

Szellőztetés indulhat, ha:

basementRH >= targetBasementRH + rhStartHysteresis

ÉS

basementAbsoluteHumidity
-
outsideAbsoluteHumidity
>= minVentilationHumidityAdvantage

ÉS

basementTemperature > minBasementTemperature

ÉS

mindkét szenzor érvényes.

Leállhat, ha:

deltaAbsoluteHumidity
<= stopVentilationHumidityAdvantage

VAGY

basementRH <= targetBasementRH

VAGY

basementTemperature <= minBasementTemperature

A minimum ON/OFF időket mindig vedd figyelembe.

==================================================
11. PÁRAMENTESÍTŐ DÖNTÉS
==================================================

Ha:

basementRH magas

ÉS

a kültéri levegő NEM alkalmas hatékony szárításra

akkor AUTO módban a páramentesítő legyen az elsődleges szárítási mód.

Példa:

pince:
17 °C
82 %

kint:
27 °C
65 %

A kinti levegő abszolút nedvességtartalma magasabb.

Eredmény:

VENTILATION = OFF
DEHUMIDIFIER = ON

==================================================
12. DÖNTÉSI PRIORITÁS
==================================================

AUTO módban:

1.
Ha RH megfelelő:

IDLE

2.
Ha szárítani kell és kültéri levegő alkalmas:

VENTILATING

3.
Ha szárítani kell és kültéri levegő nem alkalmas:

DEHUMIDIFYING

4.
Ha sensor error:

SAFE_ERROR

5.
Ha páramentesítő szükséges, de hibás:

DEHUMIDIFIER_ERROR

Alapesetben ne menjen egyszerre:

VENTILATOR
+
DEHUMIDIFIER

mert energetikailag értelmetlen lehet, hogy a páramentesítő által felmelegített/szárított levegőt azonnal kidobjuk.

Az architektúra azonban támogassa későbbi opcionális BOTH mód bevezetését.

==================================================
13. KOMPRESSZORVÉDELEM
==================================================

A páramentesítő kompresszoros berendezés.

Implementálj minimum OFF időt.

Ha kikapcsolt:

legalább

dehumidifierMinimumOffTime

ideig ne lehessen újra bekapcsolni AUTO módban.

Kerüljük a gyors:

ON
OFF
ON
OFF

ciklusokat.

Boot után se induljon azonnal újra a kompresszor, ha nincs megfelelően rekonstruálható előző állapot.

Legyen konzervatív fail-safe működés.

==================================================
14. PÁRAMENTESÍTŐ HATÁSOSSÁGÁNAK FIGYELÉSE
==================================================

A rendszer ne csak az elektromos feedbacket figyelhesse.

Legyen opcionális performance monitor.

Ha például:

dehumidifier ON

és

2-3 órán keresztül

a pince abszolút nedvességtartalma / RH-ja nem mutat értelmes javulást,

akkor generálható:

DEHUMIDIFIER_INEFFECTIVE

warning.

Ez NE feltétlenül kapcsolja le automatikusan a gépet.

A UI jelezze:

"Páramentesítés nem hatékony"
"Ellenőrizd a tartályt / készüléket / vízelvezetést"

A küszöbök konfigurálhatók legyenek.

==================================================
15. TARTÁLY BECSLÉS
==================================================

A valódi rendszer nem feltétlenül fogja közvetlenül mérni a víztartály szintjét.

De a szoftver támogasson becsült vízmennyiséget:

estimatedTankLevelLiters

A becslés:

- opcionális
- kizárólag tájékoztató
- nem használható elsődleges biztonsági érzékelésre

Legyen konfigurálható:

tankCapacityLiters = 5.0

A UI-ban megjelenhet például:

"Becsült tartály: 3.2 / 5.0 L"

de egyértelműen jelöld, hogy:

ESTIMATED

Legyen:

RESET / EMPTY TANK

touch gomb, amellyel a felhasználó ürítés után nullázhatja a becslést.

==================================================
16. DEHUMIDIFIER MONITOR ÁLLAPOTOK
==================================================

Definiálj valami ilyesmit:

enum class DehumidifierFeedback {
    Unknown,
    Off,
    Starting,
    Running,
    NoPowerDraw,
    Fault
};

A konkrét hardveres monitor később valósulhat meg.

A szimulátorban azonban teljesen működjön.

==================================================
17. FŐ VEZÉRLŐÁLLAPOTOK
==================================================

Javasolt:

IDLE

VENTILATING

DEHUMIDIFYING

MANUAL_VENTILATION

MANUAL_DEHUMIDIFIER

SENSOR_ERROR

DEHUMIDIFIER_ERROR

BASEMENT_TOO_COLD

WAITING_MINIMUM_OFF_TIME

Opcionális:

VENTILATING_AND_DEHUMIDIFYING

==================================================
18. SZENZORHIBÁK
==================================================

Ha bármely fontos környezeti szenzor:

- nem válaszol
- NaN
- irreális érték
- stale
- timeout

akkor AUTO módban:

VENTILATION = OFF
DEHUMIDIFIER = OFF

és:

SENSOR_ERROR

Manuális üzem továbbra is engedélyezhető,
de jól látható figyelmeztetéssel.

==================================================
19. KIJELZŐ
==================================================

Készíts LVGL UI-t.

Főképernyő:

PINCE                      KINT

14.7 °C                    5.3 °C
78 %                       91 %

9.6 g/m³                   6.2 g/m³

DP 10.9 °C                 DP 4.0 °C


Δ nedvesség:
+3.4 g/m³


AKTUÁLIS MÓD:

SZELLŐZTETÉS


Ventilátor:
● BE

Páramentesítő:
○ KI


Alul:

[AUTO]
[VENTI]
[PÁRAM.]
[OFF]

==================================================
20. PÁRAMENTESÍTŐ UI
==================================================

Legyen külön oldal:

PÁRAMENTESÍTŐ

Command:
ON

Feedback:
RUNNING

Üzemidő:
01:42

Teljesítmény:
310 W

Tartály:
3.1 / 5.0 L
ESTIMATED

[ TARTÁLY ÜRÍTVE ]

Ha hiba:

⚠ PÁRAMENTESÍTŐ HIBA

"Nincs érzékelt működés"

"Ellenőrizd:
- víztartály
- készülék
- tápellátás"

==================================================
21. SETTINGS
==================================================

Touch UI-ról lehessen állítani:

- target RH
- RH hiszterézis
- emergency RH

- ventiláció start ΔAH
- ventiláció stop ΔAH

- minimum pincehőmérséklet

- ventilátor minimum ON
- ventilátor minimum OFF

- páramentesítő minimum ON
- páramentesítő minimum OFF

- dehumidifier startup grace period

- tank capacity

- dehumidifier ineffective timeout

- dehumidifierAutoRestartSupported

Legyen:

SAVE

RESTORE DEFAULTS

==================================================
22. HISTORY
==================================================

Tároljuk például percenként:

timestamp

basement T
basement RH
basement AH
basement DP

outside T
outside RH
outside AH
outside DP

delta AH

ventilation state

dehumidifier command

dehumidifier feedback

estimated tank level

controller state

Legalább 24 órás RAM ring buffer.

Ne írjuk percenként feleslegesen a flash-t.

==================================================
23. WI-FI
==================================================

Wi-Fi nélkül is teljesen működjön.

Wi-Fi csak kiegészítő funkció.

Készíts egyszerű mobilbarát HTTP UI-t.

Mutassa:

- aktuális adatokat
- aktuális állapotot
- ventilátort
- páramentesítőt
- hibákat
- konfigurációt

==================================================
24. REST API
==================================================

GET /api/status

például:

{
  "basement": {
    "temperature": 14.7,
    "humidity": 78,
    "absoluteHumidity": 9.6,
    "dewPoint": 10.9
  },

  "outside": {
    "temperature": 5.3,
    "humidity": 91,
    "absoluteHumidity": 6.2,
    "dewPoint": 4.0
  },

  "deltaAH": 3.4,

  "ventilation": {
    "command": true
  },

  "dehumidifier": {
    "command": false,
    "feedback": "OFF",
    "estimatedTankLiters": 2.3
  },

  "mode": "AUTO",
  "state": "VENTILATING"
}

Továbbá:

POST /api/mode

POST /api/config

POST /api/tank/reset

==================================================
25. SZIMULÁTOR
==================================================

Ez kiemelten fontos.

Készíts natív PC-n futó szimulátort.

NE igényeljen:

ESP32-t
LVGL-t
valódi szenzort
230 V-ot

A szimulátor ugyanazt a DryingController core logikát használja, mint az ESP32.

==================================================
26. PINCEMODELL
==================================================

Kiinduló adatok:

area = 12 m²

height = 2.2 m

volume ≈ 26 m³

ventilationAirflow ≈ 100 m³/h

ACH ≈ 3.8 / hour

Modellezd:

- levegő hőmérsékletét
- vízgőz tömegét
- abszolút nedvességet
- ebből RH-t
- falakból érkező nedvességet

Legyen paraméter:

moistureGenerationGramPerHour

A nedvességváltozást tömegmérleggel modellezd.

Ne közvetlenül RH-t interpolálj.

==================================================
27. SZELLŐZTETÉS SZIMULÁCIÓJA
==================================================

Ventilátor esetén:

dm/dt =
Q * (rhoOutside - rhoBasement)
+
moistureSource

ahol:

Q:
levegő térfogatáram

rho:
vízgőz sűrűség [g/m³]

==================================================
28. PÁRAMENTESÍTŐ SZIMULÁCIÓJA
==================================================

Implementálj:

DehumidifierModel

paraméterek:

ratedExtractionLitersPerDay

effectiveExtractionLitersPerHour

tankCapacityLiters = 5.0

tankLevelLiters

compressorRunning

powerWatts

minimumOnTime

minimumOffTime

startupDelay

A vízkivonás függjön legalább közelítőleg:

- pince RH-tól
- pince hőmérséklettől

Ne legyen konstans minden körülmény között.

Egyszerű empirikus modell elfogadható, ha dokumentált.

==================================================
29. TARTÁLY SZIMULÁCIÓJA
==================================================

A kivont víz kerüljön a virtuális tartályba:

tankLevel += removedWater

Ha:

tankLevel >= 5.0 L

akkor:

tankFull = true

compressorRunning = false

powerDraw jelentősen csökkenjen

DE

a vezérlő konnektor-commandja maradhat ON.

Ez pontosan azt szimulálja, amikor:

ESP32 azt hiszi:
"páramentesítő bekapcsolva"

de a készülék tele tartály miatt valójában leállt.

A vezérlőnek ezt fel kell tudnia ismerni a monitor segítségével.

==================================================
30. TARTÁLYÜRÍTÉS SZIMULÁCIÓ
==================================================

Legyen szimulált esemény:

EMPTY_TANK

amely:

tankLevel = 0
tankFull = false

Ezután, ha a vezérelt konnektor még ON és a gép támogatja az automatikus restartot, a készülék megfelelő késleltetés után újraindulhat.

==================================================
31. PÁRAMENTESÍTŐ AUTOMATIKUS RESTART SZIMULÁCIÓ
==================================================

Legyen modellparaméter:

autoRestartAfterPowerRestore = true/false

Teszteljük mindkét esetet.

Ha false:

a konnektor újra bekapcsolása önmagában ne indítsa el a kompresszort.

A vezérlő ezt érzékelje és jelezze.

==================================================
32. SZIMULÁCIÓS FORGATÓKÖNYVEK
==================================================

Készíts legalább:

SCENARIO 1
Téli szárítás

Pince:
14 °C
80 %

Kint:
5 °C
90 %

Várható:

VENTILATION ON
DEHUMIDIFIER OFF


SCENARIO 2
Nyári párás idő

Pince:
16 °C
75 %

Kint:
28 °C
65 %

Várható:

VENTILATION OFF
DEHUMIDIFIER ON


SCENARIO 3
Száraz pince

Pince:
15 °C
58 %

Várható:

minden OFF


SCENARIO 4
Nagyon nedves pince és még nedvesebb kültér

Pince:
85 %

Kint:
nagyobb abszolút nedvesség

Várható:

DEHUMIDIFIER ON


SCENARIO 5
Páramentesítő tartály megtelik

tank:
4.9 L

A gép vizet termel.

Várható:

tank eléri az 5 L-t

compressor leáll

command továbbra is ON lehet

monitor érzékeli:

NO_POWER_DRAW

controller:

DEHUMIDIFIER_ERROR


SCENARIO 6
Tartály kiürítése

EMPTY_TANK

Várható:

hiba megszűnik

ha auto restart támogatott:
gép újraindul


SCENARIO 7
Nincs power-loss restart

autoRestartAfterPowerRestore = false

Várható:

vezérelt konnektor ON

de nincs tényleges működés

controller figyelmeztet


SCENARIO 8
Kompresszorvédelem

Gyorsan változó RH / kültéri feltételek.

Bizonyítsuk, hogy:

minimumOffTime miatt nem indul újra túl hamar.


SCENARIO 9
Szenzorhiba

Outdoor sensor offline.

Várható:

SAFE_ERROR
mindkét automatikus kimenet OFF


SCENARIO 10
Hirtelen időjárásváltozás

24 órás időjárási profil.

Napközben:
meleg, nedves

éjjel:
hűvös, szárazabb

A rendszer automatikusan váltson:

DEHUMIDIFIER
->
VENTILATION


SCENARIO 11
5 liter / nap vízkivonás

Állíts be olyan nedvességterhelést, amely mellett a rendszer téli körülmények között kb. 5 liter/nap vízmennyiséget kezel.

Vizsgáljuk meg:

- hány órát megy a ventilátor
- hány órát a páramentesítő
- mikor telik meg a tartály


SCENARIO 12
Páramentesítő hatástalan

Command ON

Power feedback szerint RUNNING

de a pince nedvessége nem csökken.

Várható:

DEHUMIDIFIER_INEFFECTIVE warning

==================================================
33. CSV OUTPUT
==================================================

Export:

timestamp

basementTemp
basementRH
basementAH
basementDewPoint

outsideTemp
outsideRH
outsideAH
outsideDewPoint

deltaAH

ventilationCommand

dehumidifierCommand

dehumidifierFeedback

dehumidifierPowerW

tankLevelLiters

tankFull

controllerState

==================================================
34. PYTHON PLOT TOOL
==================================================

tools/plot_simulation.py

matplotlib segítségével külön grafikonokon jelenítse meg:

1.
hőmérsékletek

2.
RH

3.
abszolút nedvesség

4.
ΔAH

5.
ventilátor állapot

6.
páramentesítő állapot

7.
víztartály szint

8.
páramentesítő teljesítmény

==================================================
35. UNIT TESTEK
==================================================

Teszteld:

psychrometrics

start ventilation

stop ventilation

start dehumidifier

stop dehumidifier

ventilation vs dehumidifier selection

RH hysteresis

AH hysteresis

minimum ventilation ON

minimum ventilation OFF

minimum dehumidifier ON

minimum dehumidifier OFF

compressor restart protection

sensor timeout

NaN

invalid readings

tank full

power draw missing

auto restart unsupported

tank reset

manual modes

config persistence

reboot

==================================================
36. HARDWARE ABSTRACTION
==================================================

A core controller csak ilyen inputokat kapjon:

ClimateReading basement

ClimateReading outside

DehumidifierFeedback dehumidifierFeedback

timestamp

Configuration

és adjon vissza például:

ControllerOutput {
    bool ventilationRequested;
    bool dehumidifierRequested;

    ControllerState state;

    ReasonCode reason;
}

Ne tudjon semmit:

ESP32-ről
SHT31-ről
reléről
LVGL-ről
Wi-Fi-ről

==================================================
37. FAIL-SAFE
==================================================

Boot alatt:

VENTILATOR = OFF

DEHUMIDIFIER = OFF

amíg:

- konfiguráció nincs betöltve
- szenzoradatok nem érvényesek
- controller nincs inicializálva

Relay boot glitch ne tudjon véletlenül bekapcsolni hálózati fogyasztót.

Dokumentáld:

- active high / active low
- ESP32 boot GPIO problémák
- output initialization

==================================================
38. LOGGING
==================================================

Példa:

[08:15:00]

Basement:
T=14.7
RH=78.2
AH=9.64
DP=10.9

Outside:
T=5.3
RH=91.0
AH=6.21
DP=4.0

DeltaAH=3.43

Mode=AUTO

ControllerState=VENTILATING

Ventilation:
Command=ON

Dehumidifier:
Command=OFF
Feedback=OFF
TankEstimate=2.3L

Reason:
OUTDOOR_AIR_DRYER

==================================================
39. FEJLESZTÉSI FÁZISOK
==================================================

PHASE 1

Repo skeleton

Core climate types

Psychrometrics

Configuration

Unit tests


PHASE 2

DryingController

Ventilation logic

Dehumidifier selection logic

State machine

Tests


PHASE 3

Dehumidifier model

Tank model

Feedback model

Compressor protection

Tests


PHASE 4

Complete PC simulation

Scenarios

CSV

Python plots


PHASE 5

ESP32 sensor HAL

Relay HAL

Dehumidifier monitor HAL


PHASE 6

LVGL UI


PHASE 7

Settings persistence


PHASE 8

Wi-Fi

REST API

Web UI


PHASE 9

Final validation

Documentation

Safety review

==================================================
40. MUNKAMÓDSZER
==================================================

Minden phase után:

1. build
2. unit tests
3. simulation tests
4. hibák javítása
5. csak sikeres állapot után következő phase

Ne hagyj TODO-t az aktuális phase követelményeire.

Ha valamelyik hardver részlet nem ismert:

NE találgasd.

Jelöld:

NEEDS HARDWARE VERIFICATION

és izoláld:

board_config.h

vagy megfelelő hardverabstrakció mögé.

==================================================
41. VALIDÁCIÓ
==================================================

A fő funkciók elkészülte után végezz fuzz / property based jellegű tesztelést.

Generálj legalább 10 000 véletlen állapotot:

Outdoor:
T = -15 ... +38 °C
RH = 20 ... 100 %

Basement:
T = 5 ... 25 °C
RH = 40 ... 100 %

Ellenőrizd:

- soha ne szellőztessen olyan esetben, amikor ezzel egyértelműen növeli a pince abszolút nedvességét

- sensor error esetén AUTO kimenetek OFF

- ne ciklizáljon túl gyorsan a kompresszor

- minimum ON/OFF idők teljesüljenek

- tank full állapot felismerhető legyen

- NaN ne okozzon hibás outputot

- timer rollover ne okozzon hibát

- reboot ne eredményezzen veszélyes outputot

- manual és AUTO state transitionök konzisztensek legyenek

Ha hibát találsz:

1. készíts reprodukáló tesztet
2. javítsd a kódot
3. futtasd újra a tesztet
4. tartsd meg regression testként

==================================================
42. ELSŐ FELADAT
==================================================

Első körben:

1. elemezd ezt a specifikációt
2. azonosítsd az esetleges logikai vagy architekturális problémákat
3. javasolj végleges architektúrát
4. készítsd el a repo struktúrát
5. implementáld PHASE 1-et
6. buildeld
7. futtasd a teszteket
8. mutasd meg röviden az eredményeket

Ha PHASE 1 sikeres, folytasd PHASE 2-vel.

A végső cél nem demonstrációs kód, hanem egy valós, tesztelt, szimulálható és később ESP32 hardveren futtatható pince-szárítás vezérlőrendszer.