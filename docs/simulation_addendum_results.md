# Szimulációs kiegészítés – 0.3.0

Az aktuális baseline és minden érzékenységi lista a `config/baseline.ini` fájlban módosítható. A készülék típusa, gyári vízkivonása, fogyasztása és a ventilátor tényleges légszállítása nem ismert. A 0,21 L/h referencia (feltételezett 15 °C / 80% RH), a 300 W, a 100 m³/h és a termikus paraméterek **szimulációs feltételezések**. Az 5 L / körülbelül 24 óra megfigyelés nem gyári teljesítményadat.

## Fizikai modell

A pince térfogata 26,4 m³. Az állapot vízgőztömeg grammban és hőmérséklet; az RH mindig ezekből származik. A mérleg: kezdeti víz + falforrás + beáramló víz − kiáramló víz − tartályba kivont víz − kicsapódás = végső légtéri víz. A légcsere exponenciális keveredést használ; a kültéri vízgőzsűrűséget a pince hőmérsékletére normalizálja. A vezérlő vízgőznyomás-védelme továbbra is aktív.

A kivonás a pillanatnyi T és RH függvénye, a referenciaértékre normalizált, konfigurálható hatványfüggvényekkel. Az RH-függő kivonási differenciálegyenletet a modell adott hőmérsékleten analitikusan integrálja, a felső teljesítménykorláttal és az alsó RH-határral együtt. A tartálytelítődés időpontját a lépésen belül keresi meg. Az 5 L-es tartály megállítja a kompresszort; a konnektor parancsa ettől még BE lehet.

A forrás és légcsere fél lépésben, a kivonás teljes lépésben, majd a forrás és légcsere újabb fél lépésben fut. A hőmodell kapacitása az effektív épülettömeg és a levegő hőkapacitásának összege. Fal/talaj és szellőzés hőátadása, illetve a készülék tényleges intervallum-fogyasztása szerepel benne. A készülék kondenzációs hője kapcsolható; alapból KI. A passzív kicsapódás látens hője nincs modellezve. A fal egyetlen rögzített hőmérsékletű határ, nem részletes épületmodell.

Külön ütemezés: fizikai lépés legfeljebb 10 s, vezérlő 60 s, CSV alapból 60 s. A CSV-sűrítés nem változtatja meg a döntési frekvenciát. A pillanatnyi végponti teljesítmény és az intervallum átlagteljesítménye külön oszlop. Az energia a tényleges kompresszoridőből és a fennmaradó készenléti időből adódik. A ventilátor ismeretlen teljesítménye miatt a megjelölt energia részleges; a 0 kWh ilyen sorokban nem nulla rendszerfogyasztást jelent.

## Kísérletek és eredmények

950 futás készült: 5 baseline, 900 teljes faktoriális kombináció (4 légszállítás × 5 forrás × 5 kivonás × 3 RH-cél × 3 minimumhőmérséklet), 5 nedves kivonás-, 10 teljesítmény-, 27 termikus és 2 látenshő-vizsgálat, valamint egy 30 órás szemléltető megfigyelésillesztés. A többi 949 futás hét napos. Az időjárás sima téli profil. A nedves érzékenységi csoportokban a napi tartályürítés explicit feltétel; a baseline-ban és a 900-as rácsban nincs automatikus ürítés.

| Forrás g/h | Átlag RH % | Minimum °C | Ventilátor h | Kompresszor h |
|---:|---:|---:|---:|---:|
| 50 | 65,62 | 13,23 | 34,87 | 31,18 |
| 100 | 78,11 | 10,49 | 165,93 | 0 |
| 150 | 83,08 | 10,49 | 167,50 | 0 |
| 200 | 88,01 | 10,49 | 167,92 | 0 |
| 250 | 92,30 | 10,49 | 167,92 | 0 |

**Feltárt szabályozási korlát:** a szellőztetést előnyben részesítő jelenlegi stratégia magasabb forrásnál nem biztosítja az RH-célt, bár a kültéri levegő szárító irányú. A kiegészítés kérésének megfelelően a döntési szabályt nem módosítottuk. A táblázat nem valós pince előrejelzése.

200 g/h nedves terhelés és napi ürítés mellett 0,10 / 0,15 / 0,21 / 0,30 / 0,40 L/h referenciával az átlag RH rendre 89,95 / 77,01 / 68,61 / 68,29 / 68,19%. A hasonló összes kivont víz forráskorlátból ered: a szükséges üzemidő és az RH különbözik.

Az 500 W-os nedves esetben az átlag RH 79,11%, a kicsapódás 6,68 L. A többletfűtés szellőztetési döntést tesz lehetővé, az ezt követő hűlés és folyamatos forrás emeli az RH-t. Ez erősen feltételezésfüggő vezérlési kölcsönhatás; nem készülékválasztási ajánlás.

## Kalibráció

A `calibration-observed-only.ini` csak a valóban megadott 5 L / 24 h adatot tartalmazza. Ez legalább 0,2083 L/h időátlagos kivonást jelent a megfigyelési intervallumban. Tényleges kompresszoridő, klíma, légcsere, légtéri tárolásváltozás és kondenzáció nélkül nem azonosítható külön a készülékgörbe és a falforrás. A program a hiányt figyelmeztetéssel jelöli, nem talál ki mérést.

A külön példa feltételezett 212 g/h forrással és 15 °C / 80% RH kezdeti állapottal 23,79 óra alatt tölti meg a tartályt. Ez egy lehetséges paraméterezés, nem a pince kalibrációja. A JSON/CSV illesztés nem írja felül automatikusan a konfigurációt.

## Regresszió és numerikus bizonyíték

15 CTest tesztcsoport, 95 923 C++ ellenőrzés sikeres. A korábbi randomizált állapot-, szenzorhiba-, NaN-, reboot-, reléhiszterézis- és millis-overflow regressziók megmaradtak.

Javított hibák: a végponti teljesítményből számolt intervallumenergia túlbecslése; a változó RH mellett konstansnak vett vízkivonás; valamint a fizikai és döntési időlépés összekeverése. Új tesztek vizsgálják a pontos kivonási megoldást, a startup energiáját, a tartályt, a hőmérleget és az exportfüggetlenséget.

Az 500 W-os hét napos esetben 10 / 5 / 1 másodperces fizikai lépéssel, azonos 60 másodperces vezérléssel az átlag RH 79,109672 / 79,109834 / 79,109969%; a kompresszoridő mindháromban 104,125 óra. A legnagyobb különbség 0,00030 százalékpont. A korábbi és javított konvergenciatáblák az átadott csomagban szerepelnek.

A független 950 soros CSV-audit maximális vízmérleg-eltérése 5,56e−8 g, hőmérlegmaradéka 3,77e−6 J, energiaeltérése 6,61e−11 kWh. Ez könyvelési konzisztenciát bizonyít, nem hardvervalidációt. A HTML-logikát mock DOM teszt ellenőrzi; tényleges böngészős vizuális ellenőrzés nem történt.
