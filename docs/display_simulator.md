# 2,8 hüvelykes érintős Windows-szimulátor 0.7

START.cmd vagy pince-kijelzo.exe indítja, telepítés nélkül. A bal oldalon ESP32-2432S028R-CYD 240×320 kijelző nagyított előnézete látható, helyes 3:4 aránnyal. Az egér az érintést helyettesíti. Fizikai nyomógombok és külön OLED nincsenek a tervben.

FŐOLDAL: két T/RH mérés, közérthető indok, ventilátor és páramentesítő állapota. AUTO / VENTILÁTOR / PÁRAMENTESÍTŐ / KI az érintőkijelzőn választható. KÉSZÜLÉK: parancs és visszajelzés elkülönítve, MEGTELT/NEM TELE/HIBÁS úszójel, külön MŰSZAKI ADATOK. BEÁLLÍTÁS: 18 mező három lapon, plusz/mínusz, MENTÉS és alapértékek; csak mentéskor változik az élő vezérlő, érvénytelen kombináció nem menthető. ELŐZMÉNY: a klíma idősora.

A készülék érintése nem törölheti a tele úszójelet. A jobb oldali „Próbapadi tartály ürítése” csak a virtuális tartályt üríti. A tele/hibás jel azonnal tiltja a páramentesítőt kézi módban és minimum BE időn belül is; jelhelyreállás után minimum KI marad. A ventilátor megfelelő klímánál használható.

A jobb oldalon nyolc próba, tíz hibainjektálás, T/RH bevitel, reboot, időgyorsítás és dinamikus víztömeg/hőmodell érhető el. Az idő alapból áll: Tél +10 perc = szellőztetés; Nyár +10 perc = páramentesítő táp BE. Árammérő nincs a kosárban, a működés alapból NEM IGAZOLT. A modellmérés külön kapcsolható a próbapadon; új próba kiválasztásakor kikapcsol. A pincei RS485 adatvesztés injektálása AUTO-ban mindkét kimenetet leállítja; nem fizikai buszszimuláció.

Az RH dinamikus módban a vízgőz tömegéből származik. Rögzített klímánál a T/RH tesztbemenet marad. A fizikai modell 26,4 m³ pincét, 100 m³/h légcserét, 100 g/h forrást, feltételezett 0,21 L/h kivonást és 300 W fogyasztást, 5 L tartályt használ. Ezek modellparaméterek, nem új gyári adatok.

Reboot: AUTO, KI kimenetek, új minimum KI, megőrzött fizikai tartály és RAM-konfiguráció; history és becslés nullázódik. Kilépéskor az egyedi beállítások elvesznek, NVS nincs. A Windows GDI UI nem a kész CYD firmware; az adapterek/pinek/touchkalibráció még hátra vannak.

20 CTest csoport sikeres, köztük az érintési hitboxok, beállításmentés, tartálytiltás, reboot és fizika. A régi OLED kód csak történeti regresszióként marad, a program alapból az érintős kijelzőt mutatja.

```powershell
.\pince-kijelzo.exe --render home.png 0 0 300
.\pince-kijelzo.exe --render tank.png 5 1
```

Argumentum: PNG út, próba index 0–7, oldal 0–4 (4: műszaki), opcionális további másodperc. Kép: 240×320.
