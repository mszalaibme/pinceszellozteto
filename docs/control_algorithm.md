# Képletek és állapotgép

Magnus-forma víz feletti telítésre, T Celsiusban, e hPa-ban:

```
es = 6.112 * exp(17.62*T/(243.12+T))
e = es * RH/100
AH = 216.7 * e/(273.15+T)                 [g/m³]
gamma = ln(RH/100) + 17.62*T/(243.12+T)
DP = 243.12*gamma/(17.62-gamma)           [°C]
```

A Magnus-harmatpont forrása: [Vaisala AWS810, WMO Annex 4.B](https://docs.vaisala.com/r/M212417EN-N/en-US/GUID-EBE2B115-3C98-4C8C-9F2D-A2FF1EFECFCC?contentId=bajgOz~ciE0m8DkMGKkkHA). A sűrűség az ideális gáz törvényéből ered: vízgőz R≈461.5 J/(kg K), hPa→Pa és kg→g átváltással 216.7-es faktor. A nyomási enhancement factor nincs modellezve. Támogatott bemenet -20…50 °C, 0…100% RH, vízhez viszonyított RH; nulla alatt nem jégre vonatkozó dérpontot adunk.

Ez közelítő modell normál légköri nyomásra. Mérnöki célpontként körülbelül 0.5 °C DP és 1% AH képlethibával érdemes számolni a szokásos nem extrém száraz pincei tartományban; ez nem teljes tartományra hitelesített garancia, és nem tartalmazza a szenzorhibát. A tesztek képletpontokat és invariánsokat ellenőriznek, nem laboratóriumi kalibrációt. 0% RH esetén AH=0, a DP üres optional. Hibás bemenet: calculate üres optional, skalárfüggvények NaN, DP üres optional. JSON adapter majd null-ként adja vissza a nem definiált adatot.

## Döntési sorrend

1. Visszafelé mozduló monoton idő: CLOCK_ERROR, ventilátor KI, időzítő újraindul.
2. Manuális KI: azonnali KI. Manuális BE: folyamatos BE, automatikus klímavédelmek megkerülve, szenzorhiba esetén figyelmeztetés.
3. AUTO: bármely hibás, jövőidejű vagy legalább sensorTimeout korú mérés esetén SENSOR_ERROR és azonnali KI.
4. T <= minimum: azonnali KI.
5. ΔAH <= stopHumidityAdvantage: azonnali KI, minimum ON felülírva.
6. Ha már BE: RH <= cél−rhHysteresis esetén minimum ON letelte után KI; addig MinimumOnTimer. Egyébként Drying, a két AH küszöb között is.
7. Ha KI: csak RH > cél, ΔAH >= indulási küszöb, megfelelő hőmérséklet és minimum OFF letelte esetén BE.

RH leállási alapérték 62%, újraindulás >65%. A 10 perc minimum BE csak a normál RH-cél elérés miatti leállást késlelteti. A nedvesebb levegő befúvását és túlzott hűtést nem tartja fenn. Minden valódi relékérés-változás újraindítja a megfelelő időzítőt; ugyanazon állapot ismételt kiértékelése nem. Induláskor az első update idejétől kivárjuk az 5 perc minimum KI időt. Módváltás a következő update-ben hatályos; manuális mód nem várja meg az időzítőket.

Emergency RH nem jogosít fel vak szellőztetésre. Külön highHumidityWarning jelző működik az állapot mellett. Túl nedves kültérnél explicit üzenet; KI mellett dehumidificationSuggested. A relé meghibásodását a szoftveres fanRequested nem tudja bizonyítani.

## Fizikai korlát

A specifikáció szerinti g/m³ különbség hőmérsékletfüggő; felmelegítéskor ugyanazon kültéri levegő térfogata változik. A pontosabb jövőbeli döntéshez keverési arány/vízgőznyomás és a pincehőmérsékletre átszámolt kültéri AH használható. Itt a kért AH-algoritmus fut; falhőmérséklet, felületi kondenzáció és tényleges páramentesítési teljesítmény nem következik önmagában a léghőmérsékletből.
