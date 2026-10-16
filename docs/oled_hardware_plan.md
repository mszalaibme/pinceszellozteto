# OLED, két gomb, távoli pincei mérés és úszókapcsoló

A 0.6 változat a 3,5 hüvelykes érintőpanel helyett külön ESP32-C3 és 1,3 hüvelykes SH1106 OLED mellett készül. A Windows program az ennek megfelelő hatoldalas kijelzőt és két gombot mutatja. A vezérlő önállóan dolgozik; beállítást csak szükség esetén módosítunk.

## Tervezett elhelyezés

- Központ: ESP32-C3, OLED, MCP23017, két gomb. A kültéri DF-SEN0385 saját kb. 1 m-es vezetéke ide érkezik. A kültéri szonda árnyékolt, esőtől védett, szellőző helyen mérjen; az 1 m-es I2C kapcsolatot tényleges kábelkapacitással, órajellel és felhúzókkal ellenőrizni kell.
- Pince: második ESP32-C3, SHT30-M, MAX3485. Az SHT30 a helyiség jellemző levegőjét méri, a lépcső alján, ha ott nem az ajtó légárama dominál. Ne érjen hideg falhoz, ne mérje a tápegység/panel hőjét vagy a készülékek kifúvását. Rövid helyi I2C, 5–6 m RS485 a központig.
- Két MAX3485 adóvevő: egy-egy a központnál és a pincei csomópontnál. A szenzor önmagában nem tud RS485-öt; ezért kell a második kis vezérlő.
- Ventilátor 3–4 m, páramentesítő 6–7 m távolságra maradhat. A hálózati kapcsolóelemek külön hálózati dobozban vannak; a gyengeáramú kábelek nem hálózati tápvezetékek.

## Kisfeszültségű táp és adat

Egy 5 V / 2 A USB-A tápegység és USB-A–C kábel táplálja a központot. Az 5 V külön leágazása jut a pincei csomópont 5 V bemenetére; a 3,3 V-os szenzorokat és adóvevőket a megfelelő helyi 3,3 V ág táplálja. A relémoduloknak külön STDN-3A24-ADJ készül, 3,3 V-ra beállítva és műszerrel ellenőrizve; így nem a kis ESP32 panel stabilizátorára terheljük a két tekercset. GPIO nem táplál tekercset. A buck minimum bemenete ehhez a kimenethez 4,5 V, tehát a helyi 5 V megfelelő. A központ 5 V leágazásának terhelhetőségét, a panel vezetősávját és a csatlakozást meg kell vizsgálni; szükség esetén a leágazás a panel előtt legyen. Külső 5 V mellett ne csatlakozzon második USB-táp visszatáplálás ellenőrzése nélkül.

A 10 m réz Cat5e kábelből a tényleges pincei 5–6 m szakasz készül. Egy csavart érpár A/B, a további erek megfelelő párosításával 5 V és közös GND vezethető; párhuzamos táperes kialakítás csökkenti a feszültségesést. Ez nem Ethernet/PoE kapcsolat: hálózati eszközhöz csatlakoztatni tilos. A remote rádiója kikapcsolható; Wi-Fi nem szükséges az adatúthoz. Tényleges induláskori minimumfeszültség és kimenetkapcsolási zaj mérendő.

Az RS485 egy rövid leágazású, pont-pont busz legyen, két végén egyszeri 120 Ω lezárással. A modulokon már meglévő lezárást nem duplázzuk. Egyetlen nyugalmi előfeszítő pár szükséges, ha nincs megfelelő a panelen; a kosár két 330 Ω ellenállást tartalmaz. A MAX3485 nyitott bemeneti fail-safe tulajdonsága nem helyettesíti automatikusan a lezárt busz biasát. A modul konkrét ellenállásait és az idle feszültséget összeépítéskor ellenőrizni kell. A keretekhez CRC, sorszám és helyi frissességellenőrzés szükséges. A kommunikációs firmware még nem készült el; a szimulátor érvénytelen pincei adatot injektál a kapcsolatvesztés próbájához, nem fizikai buszszimuláció.

Az SHT30 és a kültéri SHT31 külön vezérlő I2C buszán van, ezért azonos címük itt nem ütközik. MCP23017 marad a központi gombok/kimenetek bővítéséhez; a boot érzékeny GPIO-kra nem alapozunk kimenetet. A relék bemenete külön lehúzókat kapjon, és a kimeneti latch KI értéke kerüljön be a direction beállítása előtt. Az exact board revízióhoz tartozó pin terv és bootmérés még hátra van.

## Tartály: egyszintű jel, független a működésméréstől

A P31 úszó a tartály felső részén, a túlfolyási szint alatt működjön. Rögzítése és az úszó szabad mozgása a valódi tartályon ellenőrizendő. Csak kisfeszültségű kontaktusjel; a készülék saját telitartály-védelmét nem módosítjuk. A kivehető tartályhoz oldható, fröccsenéstől védett kisfeszültségű csatlakozás szükséges; konkrét mechanikai csatlakozópár a tartály ismerete nélkül nincs kiválasztva.

Egy egyszerű kéteres NO úszóvezeték szakadását nem lehet pusztán digitális beolvasással felismerni. A tervezett felügyelt bemenethez a tartálynál elhelyezett 10 kΩ végellenállás és központi 10 kΩ felhúzó adhat külön tartományt: ép, nem tele kör kb. fél táp; zárt úszó (tele) közel 0; vezetékszakadás közel teljes táp. A rövidzár biztonságosan tele/tiltott állapotot eredményez. ADC-tartományok, szűrés, érintkezőpergés és hibaküszöbök még hardveres adaptermunka; a kosárban ellenállás és 100 nF szűrőkondenzátor szerepel. Az MCP digitális bemenete önmagában ehhez nem elég. Az úszó tényleges kapcsolási irányát beépítéskor ellenőrizzük.

A core külön `TankReading` bemenetet kap: enabled, full, valid, measuredAtMs. Engedélyezett, de hibás/elavult vagy jövőbeli jel a páramentesítőt letiltja. A tele jel minimum BE időt és kézi páramentesítést is felülír; a ventilátor megfelelő klímánál tovább használható. Ürítés és jelhelyreállás után a minimum KI idő megmarad. A kijelző gombja nem hamisíthatja meg a fizikai tele jelet. Reboot után az aktív jel ismét tilt.

Az úszó nem liter- vagy százalékmérő, és nem igazolja a kompresszor működését. Külön árammérő nincs a kosárban. A szimulátor ezért alapból `Unknown` működés-visszajelzést használ; a modellből származó mérés külön próbapadi opció. A valós szenzoradapter, NVS, SH1106 firmware-renderelő és hálózati szerelés továbbra is hátralévő feladat.

## Validáció

Az új regresszió először megbukott: tele tartályra a régi próbapad még bekapcsolva hagyta a páramentesítő kérését. A javítás után a tele jel azonnal lekapcsol, kézi módban és reboot után is. Külön teszt fedi az elavult, jövőbeli, invalid jelet, ellentmondó Running visszajelzést, normál szellőztetés folytatását és az újraindítás késleltetését. Az OLED-teszt ellenőrzi a kétgombos mentést és minden próba hat oldalának fizikai sorhosszát.

Források: [SH1106 panel](https://www.hestore.hu/prod_10047954.html), [ESP32-C3](https://www.hestore.hu/prod_10048018.html), [MAX3485](https://www.hestore.hu/prod_10051919.html), [MAX3485 gyártói adatlap](https://www.hestore.hu/prod_getfile.php?id=24089), [P31 úszó](https://www.hestore.hu/prod_10041625.html), [buck](https://www.hestore.hu/prod_10038725.html).
