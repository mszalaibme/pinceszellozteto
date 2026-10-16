# Rendszervalidáció — 0.1.1, 2026-10-16

## Vizsgált rendszer és módszer

A kiindulási változat a dev ág 77a6ec3 commitja (PHASE 1–2). Nincs ESP32 firmware, fizikai relé, hőmodell, NVS vagy mérhető falfelület. Ez core-szintű validáció, nem a teljes készülék üzembe helyezési jegyzőkönyve.

Új termékfunkció helyett regressziós tesztek és egy fizikai döntési hibajavítás készültek. A teszteket először az eredeti core ellen futtattuk: két új tesztprogram hibát jelzett. Ugyanazon tesztek és randommag a javítás után sikeresek.

`test_randomized_safety`: 20 000 állapot, mt19937 seed `0x5A17BEEF`. A generátor nyers 32 bites kimenetéből explicit skáláz, nem platformfüggő uniform_real_distributionből. Kültér -15…38 °C, 20…100% RH; pince 5…25 °C, 40…100% RH. Első 10 000 eset alapkonfiguráció, második 10 000 eset érvényes érzékeny konfiguráció (indulási Δ=0.1, leállási Δ=0.01 g/m³). Minden állapotot két történettel vizsgálunk: kikapcsolt és előzőleg téli időben már bekapcsolt ventilátor. Ez 40 000 döntési helyzet, nem 40 000 független időjárási minta.

A szárítás fizikai orákuluma: azonos közel légköri össznyomásnál x=0.622 e/(P−e) vízgőz/szárazlevegő tömegarány szigorúan nő e-vel. e_out >= e_in tehát nem szárít, függetlenül a nyers AH-k sorrendjétől. Az orákulum fizikai feltételt használ; a numerikus gőznyomáshoz ugyanaz a Magnus közelítés tartozik, így a vizsgálat nem független képlet-kalibráció. A szenzorok mérési bizonytalansága, nyomáskülönbség és turbulens lokális keveredés nem modellezett.

## Reprodukált hiba és javítás

Pince 25 °C / 100% RH, kültér 38 °C / 48% RH. Nyers AH előny ≈+0.8522 g/m³, de e_in−e_out≈−0.1542 hPa. A kültéri levegő pincehőmérsékletre hűtve már telítés feletti lenne. Az alap 0.4 g/m³ leállási küszöb miatt a régi algoritmus tovább járatott; az állítható 0.1 g/m³ indulási küszöbnél el is indult.

Javítás: AUTO-ban a hőmérsékletvédelem után, az időzítők előtt e_out >= e_in esetén azonnali KI. Nem módosítottuk az AH mező jelentését vagy az AH hiszterézis küszöbeit. Manuális BE a specifikáció szerint továbbra is tudatos felülbírálás.

| Mérőszám | Régi core | Javított core |
|---|---:|---:|
| Nedvesítő indulás, alapértékek / 10 000 | 0 | 0 |
| Nedvesítő továbbjárás, alapértékek / 10 000 | 8 | 0 |
| Nedvesítő indulás, érzékeny értékek / 10 000 | 27 | 0 |
| Nedvesítő továbbjárás, érzékeny értékek / 10 000 | 43 | 0 |
| Futó döntés és kültéri DP >= pince léghőmérséklet / 20 000 | 5 | 0 |
| Futó döntés és pince T <= minimum / 20 000 | 0 | 0 |
| Indulást engedő állapotok | 6375 | 6348 |
| Feltételezett hideg felületen kondenzációs jelzés | 300 | 268 |

Az utolsó sor szándékosan nem nulla elvárás: minden esethez feltételezett falhőmérsékletet generálunk T_pince−U(0,8) °C értékkel, majd kültéri DP >= T_fal feltételt számolunk. Ez nem mért fal, nem bizonyítja a vezérlés által okozott kondenzációt, és nem becsült valós gyakoriság. Jelzi, hogy a két légtéri szenzor alapján teljes felületvédelmet nem lehet igazolni. A szárazabb levegő a meglévő magas pincei harmatpont mellett is kondenzálhat egy nagyon hideg felületen.

## További regressziók

- Mindkét szenzoron 12 hibatípus: nem válaszol/valid=false, NaN T/RH, ±infinity, tartományon kívüli T/RH, pontos timeout, jövőidejű és befagyott régi minta. Előzőleg futó AUTO azonnal KI/SENSOR_ERROR. Valós kommunikációvesztés csak az adapter hibajelzésétől, vagy a régi minta timeoutjától érzékelhető.
- 32 bites millis határ átlépése helyes 64 bites idővel nem vált állapotot. Nyers 32 bites wrap átadása CLOCK_ERROR/KI és új minimum KI várakozás. UINT64_MAX és visszafordulás is tesztelt. Ez fail-safe, de a 32 bites órát nem bővíti helyesen: a későbbi HAL feladata.
- Reboot: új core objektum AUTO/KI állapotot és teljes minimum KI várakozást ad. A konstruktorba visszaadott konfiguráció megmarad. Ez nem NVS-visszaállítás bizonyítása: a tárolóimplementáció nincs kész. Valódi ESP resetnél GPIO boot-glitch és relépolaritás továbbra is hardvervizsgálat.
- Relé: másodpercenként kedvező/kedvezőtlen időjárás egy órán keresztül. Alapértékekkel nincs indulás az előző leállást követő 300 másodpercen belül; legfeljebb 12 felmenő él. Védelmi leállás szándékosan lerövidítheti a 10 perces BE időt. Nincs garancia a rövid BE impulzusok kizárására, és 0 minimum KI konfigurációval nincs anti-chatter garancia.
- 128 szélsőállapot-kombináció: hideg/minimumközeli/meleg pince, cél/emergency/telített RH, -15…38 °C kültér és 20/100% kültéri RH. Érvényes állapotok nem okoznak szenzorhibát, a hideg/cél alatti pince nem indul.

## Fennmaradó kockázatok és elfogadási feltételek

1. **Felületi kondenzáció:** nincs falhőmérséklet. Mért leghidegebb felület és harmatpont-margó vagy helyszíni konzervatív határ szükséges a teljes védelem igazolásához. Most csak a pince léghőmérsékletén kondenzáló kültéri levegő beengedését tiltjuk automatikusan.
2. **Túlhűtés két mérés között:** az aktuális minimum-hőmérséklet leállítás reaktív. A tesztben 10.01 °C pince, −15 °C kültér, 26 m³ és 100 m³/h mellett egy hőtároló falak nélküli, tökéletesen kevert levegőmodell T(t)=Tout+(T0−Tout)exp(−ACH*t) egy perc alatt kb. 8.46 °C-ra hűl. Ez nem a valódi pince előrejelzése, hanem ellenpélda a soha nem megy 10 °C alá állításra. A következő mérés leállítja a ventilátort. Gyors mérési ciklus, mért termikus tehetetlenség és megfelelő hőmérsékleti tartalék szükséges; a 120 s stale timeout önmagában nem hővédelem.
3. **Szenzoreltolódás és hihetően hibás adat:** NaN és invalid felismerhető; egy rossz helyen, napsütésben vagy kondenzvíztől torzítva mérő, de hihető értéket adó szenzor nem. Elhelyezés és összehasonlító kalibráció szükséges. A vízgőznyomás-különbség nulla közeli döntése nem tartalmaz mérési hibamargót.
4. **Nyomás és modell:** azonos közel légköri nyomás feltételezett. Motor tényleges légszállítása, eltömődés, légcsere, falnedvesség és reléhegesztés nem megfigyelhető a core-ból.
5. **Reboot/flash:** valós NVS, sérült rekord, flashhiba, watchdog és fizikai default OFF nem ellenőrizhető hiányzó adapterrel. Nem jelöljük sikeresnek.

Ezekhez nem adtunk hozzá kitalált szenzort, időállandót vagy új firmware funkciót. A meglévő döntési hiba javítva; a teljes rendszerre vonatkozó kondenzáció- és fagymentességi garancia nincs igazolva.

## Újrafuttatás és bizonyíték

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -V
# opcionális teljes randomminta-export Windows alatt:
build/Debug/test_randomized_safety.exe randomized.csv
# Linux alatt: build/test_randomized_safety randomized.csv
```

Az új tesztek automatikusan a CMake/CTest és a meglévő Windows/Linux CI részei. A javítás előtti és utáni CTest naplók és a 20 000 soros CSV-k a feladat átadási fájljai között találhatók. A CSV írása egy plusz fájlnyitási ellenőrzést számol, ezért a közvetlen export check-száma eggyel magasabb.

Források: [Vaisala — vízgőz, harmatpont, keverési arány](https://www.vaisala.com/en/application-note/many-faces-of-water-vapor-relative-humidity-dewpoint-mixing-ratio), [Vaisala — abszolút nedvesség definíciója](https://docs.vaisala.com/r/M211280EN-E/en-US/GUID-6454B70E-2FB0-437D-8D3E-0DA8A5AD35D2), [Espressif — 64 bites időforrás és deep sleep reset](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_timer.html).
