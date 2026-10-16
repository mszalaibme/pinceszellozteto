# Takarékos HESTORE összeállítás – 2026-10-16

**Aktuális, a felhasználó 45–55 ezer Ft-os alkatrészkeretéhez igazított kosár: bruttó 50 088 Ft, szállítás nélkül.** Nettó 39 439 Ft, ÁFA 10 649 Ft. A 135 684 Ft-os változat elvetve; megtakarítás 85 596 Ft. Rendelés és fizetés nem történt.

A kosár a megnyitott böngészőben van. A sima kosár-URL másik böngészőbe nem viszi át a tartalmat. A 17 cikkszám az eredeti 13 csoportos lista tételeit, plusz kijelződoboz-, DIN-sín- és modulösszekötő tartozékot tartalmaz; nem 17 új funkciót. Az egész forintos kereskedői kerekítés miatt az egységárból számolt és kijelzett sorösszeg kis mértékben eltérhet.

## Mit hagytunk el, és miért?

- Négy differenciális buszvégpont és a hosszú CAT6 kábelek: helyettük áthelyezett vezérlő és rövid érzékelővezetékek.
- A második drága kültéri SHT31: a pincei érzékelő ismét az eredetileg kért SHT30-M.
- TCA9548A: a pincei SHT30 címválasztásával elkerülhető a címütközés.
- PZEM mérő és a hozzá tartozó UART-szintillesztő: külön működésmérés későbbi, opcionális bővítés.
- Két 24 V-os kontaktor, tekercsmeghajtó és külön 24 V-os táp: egyetlen 230 V tekercses kontaktor marad a páramentesítőhöz, az eredeti két kis relémodullal.
- Nagy 300×200-as szekrény és távoli elektronikadobozok: kisebb központi hálózati kötődoboz és egy kijelződoboz marad.

A korábbi drágább lista az alapfunkcióhoz nem volt szükséges; a távolságok megtartására és opcionális visszajelzésre túl sok kiegészítést vett fel.

## Feltétel: a vezérlő helye változik

A vezérlő és a külön hálózati doboz a pince külső falának közelébe kerül. A pincei SHT30 kb. 30–50 cm rövid vezetékkel, a kijelző/táp hőjétől távol, szellőző helyre kerüljön; ne közvetlenül a hideg falhoz, szellőző kifúvásba vagy a páramentesítő meleg légáramába. A kültéri szondát lehetőleg a saját 1 m-es vezetékével vezessük ki védett helyre. Ha ez a geometria nem tartható, először az elhelyezést kell egyeztetni, nem újabb drága hosszabbítót kosárba tenni.

Az I2C kábelezésre nincs általános, garantált méterhatár: az órajel, kapacitás, felhúzók és zavarok is számítanak. Rövid vezetékkel indulunk, és a kész működést ellenőrizni kell. A pincei érzékelő eredeti 5–6 m-es távolságát ebben az összeállításban nem tartjuk meg.

A ventilátor 3–4 m és a páramentesítő 6–7 m távolsága maradhat, ha megfelelő hálózati tápvezetéket kapnak. A kapcsolóelemek a központban maradnak, érzékelőbuszt vagy TTL UART-ot nem kell ezekhez kivezetni. Konkrét hálózati bekötést ez a lista nem ad.

## A tényleges kosár

| Tétel | Mennyiség | Nettó egységár | Nettó sorösszeg | Szerep |
|---|---:|---:|---:|---|
| [ESP32-3248S035C-CYD](https://www.hestore.hu/prod_10050365.html) (100.503.65) | 1 | 9 903 Ft | 9 903 Ft | Kijelzős vezérlő, az eredetileg megadott panel. |
| [DF-SEN0385](https://www.hestore.hu/prod_10052470.html) (100.524.70) | 1 | 7 902 Ft | 7 902 Ft | 1 időjárásálló kültéri SHT31, saját 1 m-es kábellel; a kosárban 3 munkanap. |
| [KS 5V/2A/M](https://www.hestore.hu/prod_10038280.html) (100.382.80) | 1 | 2 014 Ft | 2 014 Ft | Egyetlen 5 V / 2 A microUSB táp. A 3,3 V-os relémodulok tápját megfelelő 3,3 V-os ágról kell adni, nem a GPIO-ról; a panel stabilizátorának terhelhetősége ellenőrizendő. |
| [JT-A1250H-4P-300](https://www.hestore.hu/prod_10047751.html) (100.477.51) | 1 | 241 Ft | 241 Ft | A panelhez ajánlott 4 pólusú kábel; a konkrét lábkiosztást ellenőrizni kell. |
| [RC-40-20/FF](https://www.hestore.hu/prod_10035719.html) (100.357.19) | 1 | 866 Ft | 866 Ft | 1 kisfeszültségű összekötő-készlet a modulokhoz; nem hálózati kábel. |
| [CJMCU-2317](https://www.hestore.hu/prod_10048961.html) (100.489.61) | 1 | 1 241 Ft | 1 241 Ft | Az eredeti listában megadott GPIO-bővítő. Ha a konkrét panelen elég szabad GPIO igazolható, később elhagyható; erre most nem alapozunk megtakarítást. |
| [STI1503](https://www.hestore.hu/prod_10044641.html) (100.446.41) | 1 | 1 442 Ft | 1 442 Ft | Külön, kisebb kijelződoboz-alap; 150×110×70 mm. Egyedi előlap/kivágás szükséges, a kivágott szerelvény IP65 védettsége nem automatikus. |
| [DIN-TS35-60](https://www.hestore.hu/prod_10051529.html) (100.515.29) | 1 | 1 442 Ft | 1 442 Ft | 1 rövid DIN-sín a kontaktor rögzítéséhez. |
| [PG9-WH](https://www.hestore.hu/prod_10042108.html) (100.421.08) | 3 | 231 Ft | 692 Ft | 3 tömszelence az 5–8 mm átmérőjű kábelekhez. Valós kábelmérettel ellenőrizendő. |
| [WAGO 221-413](https://www.hestore.hu/prod_10036983.html) (100.369.83) | 5 | 252 Ft | 1 262 Ft | 5 WAGO vezetékösszekötő, az eredeti néhány darabos igényhez. |
| [PG7-WH](https://www.hestore.hu/prod_10042101.html) (100.421.01) | 2 | 180 Ft | 360 Ft | 2 kisebb tömszelence a 3,5–6 mm kábelekhez, a külön kisfeszültségű dobozhoz. |
| [ALFA AV-2+F](https://www.hestore.hu/prod_10044534.html) (100.445.34) | 1 | 2 270 Ft | 2 270 Ft | Földelt, fedeles kapcsolt dugalj a páramentesítőhöz. |
| [SHT30-M](https://www.hestore.hu/prod_10048038.html) (100.480.38) | 1 | 858 Ft | 858 Ft | 1 pincei SHT30. A címválasztó AD ponttal másik I2C-címre állítandó, ezért nem kell TCA9548A. |
| [RELC-1CH-3V-H](https://www.hestore.hu/prod_10042172.html) (100.421.72) | 2 | 764 Ft | 1 527 Ft | 2 eredetileg kért 3–3,3 V-os, aktív magas relémodul: ventilátor kapcsolójelölt és a páramentesítő kontaktorának vezérlése. A kompresszor nem közvetlenül erről kapcsolódik. A modul teljes hálózati kialakítása és induktív terhelhetősége még ellenőrizendő. |
| [IKA20-20/230V](https://www.hestore.hu/prod_10040133.html) (100.401.33) | 1 | 4 822 Ft | 4 822 Ft | 1 Iskra IKA20-20/230V kontaktorjelölt, 230 V AC tekercs, két NO érintkező. Külön 24 V-os tápot és DC tekercsmeghajtót nem igényel. A hálózati tekercs vezérlése teljes egészében a védett hálózati térbe tartozik. |
| [STI1504](https://www.hestore.hu/prod_10046615.html) (100.466.15) | 1 | 2 048 Ft | 2 048 Ft | IP65 kötődoboz-alap a hálózati résznek; 200×155×80 mm. A mechanikai elrendezés és érintésvédelem még tervezendő. |
| [ALARM-4](https://www.hestore.hu/prod_10030746.html) (100.307.46) | 2 | 275 Ft | 549 Ft | 2 méter 4×0,22 mm² kábel a rövid érzékelő- és belső bekötésekhez, nem egyetlen 2 m-es I2C-szakasz előírása. Nem 230 V-os tápvezeték. |


## Amit az olcsóbb változat tud és nem tud

A hőmérséklet/páratartalom alapján történő automatikus döntés és a két készülék kapcsolásának terve megmarad. **Külön teljesítmény- vagy működésmérő nincs a listában.** A vezérlő ezért valós hardveren a bekapcsolási parancsot ismeri, a kompresszor működését és a tartály tényleges telítettségét nem tudja biztosan megállapítani. A monitoradapter ilyenkor Unknown/NotAvailable állapotot adjon; a kijelző nem írhat igazolt MŰKÖDIK állapotot pusztán a bekapcsolási parancsból. A tartályszint legfeljebb becslés. A szimulátor modellből származó feedbackje nem helyettesíti a hardveres mérést.

A [Sensirion SHT3x gyártói adatlapja](https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf) 0–100% RH kiterjesztett mérési tartományt és 0x44/0x45 címválasztást ad meg. A korábbi, kereskedői 0–90%-os rövid leírásra alapozott SHT30-kizárás nem volt megalapozott. Ez nem jelent azonos pontosságot minden RH-n, és a nyitott modul nem időjárásálló. Az AD bekötését a konkrét modulon ellenőrizni kell.

A relémodul kereskedői 10 A / 250 V felirata nem önmagában motoros minősítés. A 16 W-os ventilátorhoz és a kontaktortekercshez a végleges panel-, érintkező- és szigetelési megfelelőséget ellenőrizni kell; ezt a beszerzési jelöltet nem nevezem már validált hálózati kapcsolásnak. A páramentesítő kompresszorát továbbra is külön kontaktor kapcsolja. A [gyártói Iskra tanúsítvány](https://www.iskra.eu/f/docs/Iskra-Installation-Contactors/NL-101961_DEKRA_statement_ID_132138_eng3.pdf) az IKA20 NO változatára 9 A AC-3 terhelhetőséget ad meg; a konkrét készülék indítási áramával és a helyszíni védelemmel ezt is össze kell vetni.

A panel pontos GPIO-kiosztása, a 3,3 V-os táp tartaléka, boot alatti kimenetek, kontaktor rögzítése, hálózati elválasztás és a páramentesítő áramszünet utáni újraindulása továbbra is ellenőrizendő. Ezek miatt nem adunk hozzá automatikusan újabb drága modult.

## Keret és kimaradó telepítési költségek

Az 55 000 Ft-os felső keretből **4 912 Ft marad**. Ebbe kell illeszteni a helyszín szerinti hálózati vezetéket és a még szükséges apró rögzítő-/szerelési anyagokat; ezek pontos költsége nincs igazolva. A 50 088 Ft ezért ellenőrzött kosárösszeg, nem teljes, munkadíjjal együtt garantált telepítési ár. Szállítás, villanyszerelői munkadíj és a meglévő áramkör esetleges védelmi átalakítása nincs benne. Ha a hátralévő anyagokkal az 55 000 Ft nem tartható, a tervet még rendelés előtt a kerethez kell igazítani.

A kosár aktuális tételei megtalálhatók a HESTORE-ban; a kültéri érzékelő 3 munkanapos jelzést kapott. Kész, a CYD panelhez igazolt előlap/keret nincs kiválasztva: a dobozon saját kivágás és rögzítés szükséges.
