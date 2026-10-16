# Windows OLED kijelzőszimulátor 0.6

A START.cmd vagy pince-kijelzo.exe indítja. Telepítés, Python és hardver nem kell. A bal oldali készülék 1,3 hüvelykes, 128×64 képpontos SH1106 OLED nagyított előnézete. A kijelző nem érintőfelület; alatta két kattintható fizikai gomb van.

## Két gomb

**KÖVETKEZŐ**: lapozás a hat oldal között. **OK**: az üzemmód/célérték/minimum hőmérséklet oldalán szerkesztés indítása, majd mentés. Szerkesztés közben a KÖVETKEZŐ a választást/értéket lépteti. A változás csak OK-val kerül a valódi C++ vezérlőhöz. A cél 40–85% között, 1%-onként, a minimum hőmérséklet 5–20°C között, 0,5°C-onként körbefordul. Érvénytelen cél >= riasztás nem menthető; a következő gombbal lépj tovább. Billentyűzethez előbb kattints a készülék előlapjára: jobbra nyíl = KÖVETKEZŐ, Enter = OK.

1. **Főoldal:** pince/kültér T és RH, ventilátor BE/KI, páramentesítő táp BE/KI, tartály- és érzékelőriasztás. AUTO akkor is figyel, amikor mindkét kimenet KI.
2. **Mi történik?**: a döntés közérthető indoka, a védelmi szünetből hátralévő perc/másodperc. A számláló a legkorábbi indulási lehetőség, akkor újra ellenőrizzük a klímát.
3. **Tartály/visszajelzés:** MEGTELT, NEM TELE vagy HIBÁS úszójel. Nem mutat fiktív százalékot/litert. Külön felirat jelzi, hogy a működés nincs igazolva.
4. **Üzemmód:** AUTO, kézi ventilátor, kézi páramentesítő, KI. OK, majd KÖVETKEZŐ, majd OK.
5. **Cél páratartalom:** menthető célérték.
6. **Minimum pincehőmérséklet:** menthető szellőztetési alsó határ.

Az RH a kijelzett célérték és a szenzor mérése; a dinamikus modell továbbra is a vízgőz tömegét számolja. A kijelző nem lineárisan mozgatja az RH-t.

## Próbapad

A jobb oldalon szerkeszthető a két klíma, választható nyolc előre beállított próba, tíz hibajelzés, időgyorsítás, +1 perc / +10 perc / +1 óra, reboot, tartályürítés és dinamikus fizika. Az idő alapból áll. A Tél próbán +10 perc után szellőztet; Nyáron +10 perc után a páramentesítő tápját kapcsolja.

**A kosárban nincs árammérő**, ezért a működés-visszajelzés alapból Unknown. A táp BE nem bizonyítja a kompresszor működését. A „Modell működésmérése” jelölőnégyzet a próbapadon bekapcsolja a virtuális mérést a régi feedback-esetekhez. Ez nem új fizikai alkatrész. Új próba kiválasztása kikapcsolja ezt az opciót.

**Tele tartály**: a külön úszójel azonnal kikapcsolja a páramentesítőt, minimum BE idő és kézi kérés ellenére is. A ventilátor alkalmas kültéri klímánál használható. A készülék OK gombja nem törölheti a fizikai tele jelet. A jobb oldali „Próbapadi tartály ürítése” gomb a virtuális tartályt üríti és megszünteti a tele injektálását. Ezután is érvényes a kompresszor minimum KI ideje. A jel reboot után is megmarad.

**Úszójel hibás / vezetékszakadás**: a felügyelt bemenet hibás jelét injektálja, letiltja a páramentesítőt. A valós kéteres NO érintkező önmagában nem ismeri fel a szakadást; ehhez a tervezett végellenállásos ADC adapter szükséges. **Pince RS485 kapcsolat**: az érvényes pinceadat elvesztését injektálja, AUTO-ban mindkét kimenet KI. Nem fizikai RS485 jel- vagy CRC-szimuláció.

A „Korábbi érintős műszaki nézet” jelölőnégyzet a korábbi 320×480-as prototípust mutatja, benne 18 haladó beállítással és előzménnyel. Ez próbapadi segédnézet, a megvásárolandó OLED nem érintős. A nézet ugyanazt az élő sessiont használja.

## Fizika, reboot, készültség

Rögzített klímánál a T/RH tesztbemenet, a készülék és a tartály órája fut. Dinamikus klímánál víztömeg- és hőmodell fut: 26,4 m³, 100 m³/h, 100 g/h forrás, 0,21 L/h referencia, feltételezett 300 W, 5 L tartály. Az utóbbi modellparaméterek nem új gyári adatok. A próbapad alján a modell által ismert tényleges tartály és kompresszoridő látható; ezek nem szivárognak az OLED-re.

Reboot: AUTO, KI kimenetek, új minimum KI idő, RAM-ban megtartott konfiguráció és fizikai tartály; history/becslés nullázása. Az alkalmazás bezárása után a szerkesztett értékek elvesznek. NVS/flash mentés, valódi SH1106 firmware, GPIO, RS485 keretkezelés és úszó ADC adapter még nem készült el. A Windows UI Win32/GDI, a vezérlés a közös DryingController C++ mag.

## Tesztek és képek

20 CTest-csoport: korábbi fizikai/random regressziók, session, tank interlock, OLED sorhossz/mentés, valódi kattintási hitbox-kezelő és GDI render. A teljes Windows OS-eseményút nincs automatizálva.

```powershell
.\pince-kijelzo.exe --render oled-home.png 0 0 300
.\pince-kijelzo.exe --render tank-full.png 5 0
.\pince-kijelzo.exe --render reason.png 1 1
```

Argumentum: PNG út, próba index 0–7, OLED oldal 0–5, opcionális további másodperc. A kép 320×480, ezen belül látható a nagyított 128×64 panel.
