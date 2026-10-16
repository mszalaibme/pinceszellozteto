# HESTORE alkatrész-összeállítás – 2026-10-16

**A böngészőben 21 tételes kosár készült. Bruttó 95 432 Ft, szállítás nélkül. Rendelés/fizetés nem történt.** A kosár a megnyitott böngésző munkamenetéhez kötődik; a sima kosárhivatkozás másik böngészőben nem viszi át a tartalmat. Az árak és készletjelzések az ellenőrzés idején érvényesek. A HESTORE egész forintos megjelenítése miatt a kerekített egységárakból számolt összeg 1 Ft-tal eltérhet a kijelzett sorösszegtől.

Ez előzetes beszerzési kosár, **nem teljes, jóváhagyott telepítési készlet és nem kész kapcsolási terv**. A hálózati védelmek, kábelhosszak, csatlakozók, nyák- és dobozrögzítések véglegesítése még hátra van. A jelenlegi kosár elemei a katalógusban megtalálhatók; a két SHT31-nél a kosár 3 munkanapos beszerzést jelzett. A háztartási páramentesítőt és a már megadott ventilátort nem tettem újra a kosárba.

## Kapott készülékadatok

- Ventilátor: Vents 100 VKO L TURBO; a [megadott Dalap termékoldal](https://www.dalap.hu/kismeretu-golyos-csapagyazasu-csoventilator-emelt-teljesitmennyel-szukitett-o-100-mm-x11792) 230 V, 16 W, 0,1 A és 135 m³/h névleges légszállítást ad meg. A tényleges csőhálózati légszállítás ettől eltérhet.
- Páramentesítő: felhasználó által megadott adattábla: 350 W / 1,7 A névleges, legfeljebb 420 W / 2,1 A, 195 m³/h belső légkeringetés, 5–32 °C üzemi tartomány. A pontos modell és kompresszorindítási/LRA adat nincs meg. A 195 m³/h nem kültéri légcsere. Ezek nem helyettesítik a fogyasztás- és vízkivonásmérést.
- A baseline szimuláció korábbi 300 W-os feltételezését nem írtam át és nem kevertem a korábbi 950 futás eredményébe. Az új névleges adatok külön hardveradatok.
- A [gyártói Iskra termékoldal](https://b2bshop.iskra.eu/en_US/products/ikd20-20-24v40-500hz-030046024000-en) az IKD20-20/24V jelöltet azonosítja; a [gyártói tanúsítvány](https://www.iskra.eu/f/docs/Iskra-Installation-Contactors/NL-101961_DEKRA_statement_ID_132138_eng3.pdf) 9 A motoros AC-3 terhelhetőséget igazol a NO változatra. Ez alapján ésszerű jelölt a 2,1 A-es készülékhez, de nem önmagában a 20 A felirat miatt. A kompresszorindítás és a teljes környezet ellenőrzése megmarad.

## A tényleges kosár

| Tétel | Mennyiség | Nettó egységár | Nettó sorösszeg | Szerep / feltétel |
|---|---:|---:|---:|---|
| [ESP32-3248S035C-CYD](https://www.hestore.hu/prod_10050365.html) (100.503.65) | 1 | 9 903 Ft | 9 903 Ft | Kijelzős ESP32 vezérlő; a pontos board revision és GPIO-k ellenőrizendők. |
| [DF-SEN0385](https://www.hestore.hu/prod_10052470.html) (100.524.70) | 2 | 7 902 Ft | 15 804 Ft | 1 pince + 1 kültér. Két védett SHT31; 3 munkanapos beszerzési jelzés. Az olcsó SHT30-M kereskedői leírása csak 0–90% RH-t adott meg, ezért a 100%-ig vizsgált pince végleges listájába nem ezt választottam. |
| [TCA9548A-M](https://www.hestore.hu/prod_10047629.html) (100.476.29) | 1 | 567 Ft | 567 Ft | A két azonos I2C-című érzékelő külön csatornára kerülhet. Nem hosszúkábel-meghajtó. |
| [KS 5V/2A/M](https://www.hestore.hu/prod_10038280.html) (100.382.80) | 1 | 2 014 Ft | 2 014 Ft | 5 V / 2 A, microUSB: a vezérlő külön tápja. Nem a kontaktorok 24 V-os táplálása. |
| [IKD20-20/24V](https://www.hestore.hu/prod_10040139.html) (100.401.39) | 2 | 5 475 Ft | 10 950 Ft | 1 ventilátorhoz + 1 páramentesítőhöz, előzetes motoros kapcsolójelölt. 24 V AC/DC tekercs; NO érintkező AC-3/AC-7b: 9 A. A kész beépítéshez indítási áram és védelmi koordináció még ellenőrizendő. |
| [SSR-2CH-3-24V-HIGH](https://www.hestore.hu/prod_10048147.html) (100.481.47) | 1 | 1 849 Ft | 1 849 Ft | Két 24 V DC kontaktortekercs kisfeszültségű vezérlésének jelöltje. 3–24 V bemenet, legfeljebb 50 V DC / 2 A kimenet. NEM 230 V-os SSR, a készülékeket közvetlenül nem kapcsolhatja. Meghajtóáram, indulási állapot és tekercstranziens ellenőrzendő. |
| [HDR-15-24](https://www.hestore.hu/prod_10040961.html) (100.409.61) | 1 | 4 772 Ft | 4 772 Ft | 24 V, 0,63 A, 15,2 W tekercstáp jelölt. A két konkrét tekercs behúzási és tartási áramával véglegesítendő; a 230 V-os része a hálózati szekrénybe tartozik. |
| [JT-A1250H-4P-300](https://www.hestore.hu/prod_10047751.html) (100.477.51) | 1 | 241 Ft | 241 Ft | A kijelzős panelhez a kereskedő által ajánlott 4 pólusú, 1,25 mm-es kábel. Lábkiosztás/polaritás a konkrét panel alapján. |
| [RC-40-20/FF](https://www.hestore.hu/prod_10035719.html) (100.357.19) | 1 | 866 Ft | 866 Ft | Próbapadi összekötés, kizárólag kisfeszültségű oldalra. Nem végleges pincekábel és nem hálózati vezeték. |
| [GOOBAY-72227](https://www.hestore.hu/prod_10051073.html) (100.510.73) | 1 | 827 Ft | 827 Ft | MicroUSB adatkábel a programozáshoz. |
| [PZEM-004T-PZCT-WE](https://www.hestore.hu/prod_10050433.html) (100.504.33) | 1 | 6 542 Ft | 6 542 Ft | Dobozolt, nyitható áramváltós PZEM-004T működés-/teljesítmény-visszajelzéshez. TTL UART Modbus RTU, 9600 8N1; 5 V-os logikai oldal. A hálózat felől is táplálni kell; pusztán 5 V-ról nem kommunikál. A kereskedő 100 A-es CT és 23 kW mellett egy helyen 0–10 A tartományt is ír: a pontos változat és adatlap átvételkor ellenőrizendő. |
| [TXS0108E-M](https://www.hestore.hu/prod_10047917.html) (100.479.17) | 1 | 336 Ft | 336 Ft | UART 3,3/5 V jelszintillesztő-jelölt. VCCA/VCCB, OE és felhúzások a konkrét PZEM/panelhez tervezendők. Nem 230 V-os leválasztó. |
| [CJMCU-2317](https://www.hestore.hu/prod_10048961.html) (100.489.61) | 1 | 1 241 Ft | 1 241 Ft | OPCIONÁLIS MCP23017 GPIO-bővítő a szűkös panelkivezetésekhez. Nem UART-bővítő és nem I2C-címmultiplexer. Ha két megfelelő vezérlő-GPIO marad szabadon, elhagyható. |
| [STI1700](https://www.hestore.hu/prod_10048464.html) (100.484.64) | 1 | 9 022 Ft | 9 022 Ft | Hálózati szekrény alap, külön az érinthető kijelzőelektronikától; a belső elrendezés, PE-kötés és rögzítés még tervezendő. |
| [STI1503](https://www.hestore.hu/prod_10044641.html) (100.446.41) | 1 | 1 442 Ft | 1 442 Ft | Kisfeszültségű kijelződoboz alap, saját kivágással/kerettel. Kivágás után a kész szerelvény IP65 védettsége nem következik az üres doboz minősítéséből. |
| [DIN-TS35-60](https://www.hestore.hu/prod_10051529.html) (100.515.29) | 2 | 1 442 Ft | 2 885 Ft | 2 × 60 mm DIN-sín szerelési alapanyag. A teljes védelmi készülékkiosztáshoz további sín kellhet. |
| [PG9-WH](https://www.hestore.hu/prod_10042108.html) (100.421.08) | 5 | 144 Ft | 721 Ft | 5–8 mm külső kábelátmérőhöz. 5 darab összesen olcsóbb volt, mint 4 a mennyiségi lépcső miatt. A valós kábelek átmérőjével ellenőrizendő. |
| [WAGO 221-413](https://www.hestore.hu/prod_10036983.html) (100.369.83) | 10 | 158 Ft | 1 577 Ft | 10 darab 3 vezetékes összekötő. Egy darabban a három csatlakozás közös potenciálú; nem három külön áramkör. A szükséges kötés-/PE-kialakítás még tervezendő. |
| [PG7-WH](https://www.hestore.hu/prod_10042101.html) (100.421.01) | 2 | 180 Ft | 360 Ft | 3,5–6 mm külső kábelátmérőhöz, 2 darab. A szenzor tényleges kábelátmérőjével összevetendő. |
| [ALFA AV-2+F](https://www.hestore.hu/prod_10044534.html) (100.445.34) | 1 | 2 270 Ft | 2 270 Ft | Kapcsolt, földelt, fedeles 16 A / 250 V dugalj jelölt a páramentesítőnek. A kész rendszer érintésvédelmi ellenőrzést igényel. |
| [DPCB-812](https://www.hestore.hu/prod_10048010.html) (100.480.10) | 1 | 956 Ft | 956 Ft | 8×12 cm univerzális szerelőnyák, csak a kisfeszültségű elektronikához. Nem hálózati kapcsolópanel. |

## Ami nincs véglegesítve vagy nincs a kosárban

| Elem | Állapot és ok |
|---|---|
| Pontosan ehhez a 3,5 colos panelhez illő kész kijelzőkeret, tömítés, érinthető előlap | Nem találtam a panelhez igazolt kész megoldást. A kosárban általános doboz van; egyedi mechanikai munka/keret és szerelési távtartók szükségesek. A csavar-/távtartóméret CAD és a pontos board után választható. |
| Kültéri érzékelő kábelhossza, esetleges buszhosszabbító | A távolság még nincs megadva. A DF-SEN0385 kábele 1 m. A TCA9548A nem oldja meg a hosszú I2C kábel jelminőségét. Hosszabb szakasznál a buszátvitelt külön kell megtervezni; nem választottam találomra kábelt vagy extendert. |
| Vezérlő GPIO / UART és csatlakozólábkiosztás | A pontos CYD board revision és szabad kivezetések még nincsenek igazolva. A PZEM UART-hoz két használható GPIO vagy valódi UART-bővítő kell; az MCP23017 ezt nem pótolja. A kosár együttműködési terv, nem igazolt pin-kompatibilitás. |
| Kisfeszültségű kimenetek indulási védelme, OE/lehúzók, tekercstranziens-védelem | A végleges kapcsolás alapján kell meghatározni az alkatrészeket és mennyiségeket. Az SSR modul 3 V alsó bemeneti határa önmagában nem igazolja az ESP32/MCP kimenetének elegendő meghajtóáramát. |
| 230 V-os betáp-/ventilátor-/dugaljkábel, erek és keresztmetszet, érvéghüvelyek, PE/N sín, sorkapocsrögzítés | Telepítési hossz, vezetési mód és helyszíni érintésvédelem alapján méretezendők. A jumpervezetékek nem használhatók helyettük. Ezek hiánya nem azt jelenti, hogy nem kaphatók a HESTORE-ban. |
| Biztosíték/kismegszakító, áram-védőkapcsoló, leválasztás és védelmi koordináció | A meglévő pinceáramkör ismerete nélkül nem választottam B/C jelleggörbét, áramerősséget vagy RCD-t. NEEDS ELECTRICAL DESIGN / VERIFICATION, az eredeti specifikációnak megfelelően. A hálózati részt villamos szakembernek kell véglegesítenie és ellenőriznie. |
| Szellőzőcső, rács, visszacsapó és eső/nap elleni érzékelő-elhelyezés | A helyszíni légtechnikai kialakítás függvénye. A megadott ventilátor Dalap termék; nem választottam helyette számítógépventilátort. |
| Valós auto restart és teljesítmény-visszajelzés | A készülék áramszünet utáni viselkedése még kipróbálandó. A PZEM hardverjelölt, a hozzá tartozó ESP32 monitoradapter még fejlesztendő. A szimulátor nem írható fel hardverigazolásként. |

A hálózati és kisfeszültségű oldalt fizikailag külön kell tartani. A 24 V-os SSR-kimenet kizárólag a tekercskört kapcsoló jelölt; **nem kerülhet közvetlenül a 230 V-os készülék elé**. Konkrét hálózati bekötési rajzot ez a lista nem ad.

A böngésző a hivatalos fájlexportot egy külső követési kérés miatti tiltással megakasztotta; ezt nem kerültem meg. A helyi JSON és ez a táblázat a látható kosártartalomból készült saját feljegyzés, nem a bolt hivatalos exportfájlja. A kosarat a megnyitott böngészőben hagytam.
