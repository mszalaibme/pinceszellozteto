# Interaktív Windows kijelzőszimulátor

Indítás: **START.cmd** vagy **pince-kijelzo.exe**. Nincs böngésző-, Python-, ESP32- vagy LVGL-függőség. A 320×480-as kijelző 125%-os nézetben jelenik meg; kisebb asztalon az egész ablak arányosan csökken. Az egér az érintést helyettesíti.

Ez a kijelző működését bemutató natív Windows UI-prototípus. A rajzolás Win32/GDI, a döntést a meglévő **DryingController C++ mag** hozza. A firmware-re telepíthető LVGL-renderelő és a hardverillesztés későbbi munka; a Windows-kijelző nem bizonyítja azok működését.

## Amit ki lehet próbálni

- **Főoldal:** pince és kültér T/RH, számított abszolút nedvesség és harmatpont (DP), ΔAH, aktuális döntés és indok, ventilátor- és konnektorparancs. Az AUTO / VENTI / PÁRAM. / OFF gombok működnek.
- **Készülék:** parancs és külön visszajelzés, pillanatnyi szimulált mérési teljesítmény, parancs üzemideje, becsült tartály, ürítésgomb. Az ESTIMATED érték nem tényleges tartályérzékelő.
- **Beállítás:** három oldalon 18 mező, +/−, MENTÉS és ALAPÉRTÉKEK. A módosítás csak mentéskor kerül az élő vezérlőre; az érvénytelen kombinációt elutasítja. Az alapértékek visszatöltése is tervezet, mentést igényel. A gyári core-alapértékben a készülék auto restartja NEM igazolt.
- **Előzmény:** percenkénti RH- és hőmérsékletminta, legfeljebb 1441 pont. A minta kezdetben, illetve klímamódosításkor is bekerül. Azonos időpontban több próbapadi módosítás lehetséges.

A jobb oldali próbapadon külön állítható a klíma, a hibainjektálás, az idő és a fizikai szimuláció. A döntési napló a döntési indok váltását rögzíti. A próbapad alján a modell ismert tartályszintje és kompresszorideje látható: ezek nem szivárognak át a készülék kijelzőjére.

## Gyors próbák

1. Válaszd a **Tél** próbát. Induláskor mindkét kimenet KI. Nyomd meg az **+10 perc** gombot: a minimum KI idő leteltével szellőztet.
2. Válaszd a **Nyár** próbát. **+10 perc** után a konnektor BE; további **+1 perc** után a KÉSZÜLÉK oldalon RUNNING, kb. 300 W jelenik meg. Ez feltételezett készülékfogyasztás.
3. Válaszd a **Tele tartály** próbát. A próba előrelépteti az időt a hiba láthatóságához (14:01). A konnektor BE, a visszajelzés NO_POWER_DRAW. A kijelző csak tartály/készülék/táp ellenőrzését kéri, nem állít biztos hibadiagnózist. Az ürítésgomb megszünteti az injektált telítettséget és nullázza a becslést; az indulásnak újra van késleltetése.
4. Válaszd a **Szenzorhiba** próbát. AUTO-ban mindkét kimenet KI; a kézi VENTI továbbra is választható, látható figyelmeztetéssel. NaN külön is injektálható.
5. A **Restart nincs igazolva** próbában a vezérlő megtagadja a páramentesítő indítását. A BEÁLLÍTÁS második lapozása után az „Auto restart igazolt” mezővel tesztelhető a változás. Ez nem igazolja a valódi készülék képességét.

A nyolc előre beállított próba új sessiont indít: alapkonfiguráció, AUTO, alap időzítők, rögzített klíma. Az egyedi klíma alkalmazása megőrzi az időzítőket, és új fizikai pinceállapotot ad meg. A textbox értékek nem követik folyamatosan a dinamikus modellt; szerkeszthető új bemenetek maradnak.

## Idő és fizika

Alapból az idő áll. Az Indítás 1× / 10× / 60× / 600× gyorsítással lépteti; egy valós másodperc alatt ennyi szimulált másodperc fut. Az időgombok szünetben is működnek. Az UI-session vezérlője és fizikai modellje másodpercenként fut; ez az interaktív próbapad üteme, különbözik a batch konfigurálható 60 másodperces döntési ütemétől.

**Rögzített klíma** esetén a beírt T és RH külső tesztbemenet marad. Az időzítők, visszajelzés, kompresszor és tartály futnak; a pince vízmérlege ilyenkor szándékosan nem fizikai kísérlet. Ezzel pontosan kipróbálhatók az állapotok.

**Dinamikus klíma** esetén ugyanaz a víztömeg- és termikus modell fut, mint a batchben: 26,4 m³, 100 m³/h, 100 g/h forrás, 0,21 L/h empirikus referencia, feltételezett 300 W, 5 L fizikai tartály. Az RH mindig a vízgőztömegből származik. A forrás és légcsere két fél lépésben fut a kivonás körül, majd az intervallum energiája fűti a termikus modellt. A kültéri határ itt rögzített, a téli napi profil a batchben érhető el. A kijelző „tartálybecslés méret” beállítása csak a vezérlő becslését módosítja, a próbapadi készülék 5 L-es fizikai tartályát nem.

Hibainjektáláskor a NO_POWER / FAULT / tele tartály leállítja a próbapadi kompresszort; ismeretlen feedback esetén a készülék futhat, de a mérési jel nem elérhető. A tele tartály injektálása a próbapad ismert szintjét 5 L-nek mutatja, a becslést nem tölti fel. Ez tudatos teszteset, nem az addigi vízmérlegből felgyűlt víz.

**Reboot:** AUTO mód, idő nullázása, kikapcsolt kimenetek, új minimum KI és startup idők. A pince és a modellben addig összegyűlt víz megmarad; a RAM-ban szerkesztett konfigurációt a próbapad megtartja. A kijelzett tartálybecslés és history nullázódik. NVS és valódi flash-restore nincs. Az alkalmazás bezárása után a szerkesztett beállítások elvesznek.

## Tesztelés és képkimentés

`test_display_session` a valódi core időzítőit, mind a nyolc próbát, a kézi módokat, NaN-t, hibát, rebootot, ürítést és dinamikus fizikát ellenőrzi. `display_touch_navigation` közvetlenül ugyanazokat a hitbox-kezelőket hívja, mint az egér: módváltás, tabok, +/−, lapozás és mentés. `display_render` ugyanazzal a GDI-renderelővel PNG-t generál. Ezek nem automatizálják a valódi Windows-egér- és ablakkezelést.

```powershell
.\pince-kijelzo.exe --render home.png 0 0 300
.\pince-kijelzo.exe --render tank-warning.png 5 1
.\pince-kijelzo.exe --render settings.png 0 2
```

Az argumentumok: PNG útvonal, próba index 0–7, képernyő 0–3, opcionális további másodperc. A képméret 320×480. Az exportok vizuálisan ellenőrizhetők; az élő natív ablak teljes OS-eseményútja külön kézi próba.
