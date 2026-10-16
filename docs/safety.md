# Feszültségtartományok és fail-safe

**Páramentesítő kiegészítés — NEEDS ELECTRICAL DESIGN / VERIFICATION.** Kompresszoros fogyasztóhoz megfelelően méretezett kapcsolóelem/kontaktor és független elektromos terv szükséges. A kis relémodul alkalmasságát nem feltételezzük. A monitor galvanikus leválasztása és a készülék restart-képessége külön ellenőrizendő. A szoftveres minimum KI idő nem helyettesíti a készülék saját védelmét. Mindkét fogyasztónak külön hardveres default OFF kell.

LOW VOLTAGE: ESP32, TFT/touch, I2C szenzorok, relévezérlés. MAINS: reléérintkezők és 230 V ventilátor. Fejlesztés és teszt csak kisfeszültségen, szimulált terheléssel történjen. A hálózati kivitelezést megfelelően képzett szakember végezze a helyi követelmények szerint; ez nem bekötési terv.

A fanRequested=false szoftveres alapállapot nem garantál boot alatti kikapcsolást. Megfelelő hardveres OFF irányú fel-/lehúzás, nem behúzott relével nyitott ventilátoráramkör, megfelelő tápegység, motoros terhelésre minősített kapcsolóelem, burkolat és feszültségelválasztás szükséges. Active LOW modul tápfeszültsége és bemenete külön vizsgálandó; a 3.3 V logikai jel nem jelent automatikus kompatibilitást.

Az adapter majd először a kikapcsolt jelszintet tölti az output latch-be, és csak utána engedélyezi a kimenetet. Reset, bootloader, watchdog és tápfelfutás alatt az OFF-ot hardveresen is biztosítani kell. Hegesztett reléérintkezőt szoftver nem tud lekapcsolni. Manuális BE felülírja az automatikus szenzor-/hőmérséklet-/nedvességvédelmet; az UI-ban látható figyelmeztetés kell.

A pincének nincs közös légtere a lakóházzal. A jelen projekt nem vezérel égéstermék-elvezetést vagy légzésvédelmi rendszert. A modell alapján nem garantálható épületszerkezeti kiszáradás.
