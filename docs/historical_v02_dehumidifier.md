# Történeti 0.2 leírás – nem az aktuális modell

# Páramentesítő vezérlés és modell

## Valós eszköz követelményei

Az ESP32 csak kisfeszültségű kimenetet ad egy megfelelő motoros/kompresszoros kapcsolásra minősített relé/kontaktor felé. A konkrét kontaktor, védelem, burkolat, elválasztás és hálózati terv: **NEEDS ELECTRICAL DESIGN / VERIFICATION**. Egy általános ESP32 relémodul alkalmasságát nem feltételezzük.

Az automatikus tápvisszatérés utáni indulást a konkrét készüléken kell ellenőrizni, biztonságos körülmények között. Ha standby-ba tér vissza és POWER gombot vár, a vezérelt konnektor nem elegendő. A capability alapból false. Készülékspecifikus minimális kompresszoridőket és megengedett hőmérsékletet is ellenőrizni kell; a konfigurációban szereplő alapértékek nem helyettesítik a gyártói előírást.

Az IDehumidifierMonitor típusos feedbacket és opcionális teljesítményt ad. Lehet áram-/teljesítménymérő, intelligens konnektor vagy állapotkontakt. A mostani szimulátor ezt ténylegesen implementálja a modellből; a valós HAL-adapter még nem készült el. Unknown és stale esetben figyelmeztetünk, nem állítunk működést. Hiteles NoPowerDraw/Off vagy minimum alatti fogyasztás a grace után NO_POWER_DRAW; a belső állapot nem állít biztos tartálytelítettséget. A UI üzenet: ellenőrizd a tartályt/készüléket/tápellátást.

## Szimulált készülék

Alapértékek: 10 L/nap névleges kivonás, 310 W működés, 2 W standby, 5 L tartály, 30 s indulás, 600 s minimum kompresszor KI, 900 s minimum BE cél. A teljesítmény és kivonás nem azonos: a performanceFactor=0 forgatókönyv 310 W fogyasztás mellett nulla vizet von ki.

Az effektív kivonási becslés:

```
L/h = ratedLperDay / 24
      * clamp((RH-35)/45, 0, 1.2)
      * clamp((T-5)/15, 0, 1.3)
      * performanceFactor
```

Ez dokumentált empirikus tesztmodell, nem gyártói adat. A névleges alappont 20 °C / 80% RH. Az adott intervallumban a ténylegesen eltávolítható vízgőztömeg és a szabad tartálykapacitás korlátozza a kivonást. A víz a pincemodell extractedGram mérlegtagjából kerül a tartályba; víz nem keletkezhet a tartályban a levegőtől függetlenül.

Ha a tartály eléri az 5 L-t, a kompresszor leáll, standby teljesítmény és NoPowerDraw feedback keletkezik, a konnektor parancsa ettől nem változik. EMPTY_TANK után a készülék megtartja a kompresszor minimum KI védelmét és új indulási késleltetést alkalmaz. Power-restore=false esetén nincs kompresszorindulás pusztán a konnektortól.

A külső tápelvételt a modell nem tudja minimum BE idővel megakadályozni: a valóságban is azonnal leállna. A normál minimum BE időt a DryingController tartja; a modell külön számolja az idő előtt megszakított futásokat. Tartálytelítettség belső védelmi leállásként szintén felülírhatja a minimum BE időt.

## Becsült tartály

Alapkonfigurációban a becslés kikapcsolt. A bemutató szimuláció kifejezetten engedélyezi. A becsült 5/24 L/h nem azonos a hőmérséklet-/RH-függő tényleges kivonással, ezért a két görbe eltérhet. Kezdeti 4.9 L modelltartály mellett a controller becslése továbbra is 0-ról indul: nincs titkos hozzáférése a fizikai modelltartályhoz. Ez szándékosan szemlélteti a becslés korlátját.
