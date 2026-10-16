# DryingController — aktuális vezérlés

A pszichrometria változatlan Magnus-képleteken alapul: es=6.112 exp(17.62T/(243.12+T)), e=es*RH/100, AH=216.7e/(273.15+T). A harmatpont a Magnus inverze. Támogatott T -20…50°C, RH 0…100%; nulla RH-nál nincs véges DP. Forrás és a fizikai korlátok a system_validation.md és moisture_model.md dokumentumokban.

## AUTO

A szárítási igény RH >= target+rhStartHysteresis esetén kapcsol be; RH <= target esetén kapcsol ki. A köztes sáv megtartja az igényállapotot. Alapérték: 68% indulás, 65% leállási igény.

Érvényes/friss pincei ÉS kültéri mérés szükséges mindkét automatikus fogyasztóhoz. Valid=false, NaN, infinity, tartományhiba, jövőidejű vagy legalább 120 s korú adat: mindkét AUTO kimenet KI, SENSOR_ERROR. Visszafelé lépő óra: CLOCK_ERROR, mindkettő KI, időzítők újraindítása.

Szellőztetés akkor alkalmas, ha pince T > minimum, a kültéri gőznyomás szigorúan kisebb, és ΔAH legalább a bekapcsolási küszöb. Már járó ventilátornál a kisebb leállási küszöb érvényes. A hőmérséklet- és gőznyomás-védelem mindig szükséges, nem csak induláskor.

Ha szárítás kell és a szellőztetés alkalmas, ventilátor az elsődleges. Ha nem alkalmas (a hideg pince is ide tartozik), a páramentesítő az elsődleges, ha a restart capability ellenőrzött. Az elégtelen falnedvesség-kezelési teljesítmény vagy a készülék alacsony hőmérsékleten romló hatásfoka külön figyelmeztetéshez vezethet; nem találunk ki gyártói működési tartományt.

Már járó eszköznél normál cél-elérés/stratégiaváltás megvárja a minimum BE időt: ventilátor 10 perc, páramentesítő 15 perc. Ventilátor hideg/nem szárító levegő esetén azonnal KI. Szenzorhiba, explicit készülékfault és kézi OFF azonnali leállást ad. AUTO-ban a két kimenet soha nem igaz egyszerre. Ha az elsődleges eszköz minimum KI idejére várunk, mindkét kimenet KI; nem indítjuk el kerülő úton a másik eszközt.

Külön minimum KI idők: ventilátor 5 perc, páramentesítő 10 perc. Bootkor az első update-től számítjuk a teljes várakozást. A hibából visszatérés sem kerüli meg a valódi utolsó kimenetváltás idejét. Az időszámítás kivonásos, 64 bites, a jövőidejű mérés és visszalépés ellen védett.

## Páramentesítő diagnózis

A restart capability alapból false. Nem állítjuk, hogy egy konnektorbekapcsolás képes elindítani az ismeretlen készüléket. Unknown/stale monitor esetén feedbackUnavailable figyelmeztetés keletkezik. Hiteles Off/NoPowerDraw vagy konfigurált minimum alatti fogyasztás 2 perc startup grace után DEHUMIDIFIER_ERROR/NO_POWER_DRAW. Egy korábban már futó gép leállását a folyamatos hiány első észlelésétől számított grace után jelezzük.

NO_POWER_DRAW esetén a konnektor BE maradhat, így helyi tankürítés után képes helyreállni. Fault esetén a konnektor KI. Nem állítunk biztos TANK_FULL diagnózist dedikált szint-visszajelzés nélkül. Running feedback helyreállásakor a no-power diagnózis megszűnik.

A performance monitor folyamatosan visszajelzett működés alatt 3 órás ablak AH-javulását vizsgálja (alap 0.2 g/m³). A sikertelen javulás warning, nem leállítás. A tank estimate opcionális, nem biztonsági szabály; resetje csak a becslést nullázza.

## Manuális módok

MANUAL_OFF mindent kikapcsol. MANUAL_VENTILATION megkerüli az automatikus klímavédelmeket, szenzorfigyelmeztetés megmarad. MANUAL_DEHUMIDIFIER a klímavédelmeket megkerüli, de a kompresszor minimum KI, a restart capability és az explicit készülékfault továbbra is érvényes. BOTH mód ebben a kiadásban nincs.

A visszatérő ControllerOutput külön tartalmazza a parancsokat, állapotot, ReasonCode-ot, szenzor/feedback/hatástalansági figyelmeztetéseket és az opcionális tankbecslést. Ezek parancsok és diagnózisok; a fizikai eszköz működése csak monitorral állapítható meg.
