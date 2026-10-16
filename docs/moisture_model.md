# Víztömegmérlegen alapuló pincemodell

A `simulator/BasementModel` állapota M [g], a térfogaton belüli vízgőz tömege. V [m³] fix. Levezetett értékek:

```
AH = M/V
RH = 100 * AH / AH_sat(T_basement)
```

RH-t nem léptetünk közvetlenül, és nem interpoláljuk a beltéri/kültéri RH-t. A kezdeti RH kizárólag a kezdeti M meghatározására szolgál. A `reading()` levezetett RH-t ad a valódi VentilationController bemenetére.

## Légcsere és nedvességterhelés

Q [m³/h] a pincei állapotra vonatkozó térfogatáram, alapértéke 100; V=26 m³; G [g/h] a `moistureGenerationGramPerHour`. Tökéletes keveredést és közel azonos, állandó össznyomást feltételezünk. A bejövő levegő vízgőzsűrűségét ugyanarra a hőmérsékletre kell vonatkoztatni, mint a távozó térfogatáramot:

```
rho_in = 216.7 * e_out / (273.15 + T_basement)   [g/m³]
dM/dt = G + Q*rho_in - (Q/V)*M                  [g/h]
```

A kültéri saját hőmérsékletén számított AH közvetlen használata hibás lenne a pincei térfogatárammal. Q=0 mellett M(t+dt)=M(t)+G*dt. Q>0 esetén k=Q/V, M_eq=(G+Q*rho_in)/k és telítés alatt:

```
M(t+dt) = M_eq + (M(t)-M_eq)*exp(-k*dt)
```

Az implementáció expm1-t használ a kicsi időlépések numerikus pontosságához. Alapértékekkel ACH=100/26≈3.846/h. A hőmérséklet egy lépésen belül rögzített; az eltelt idő explicit másodpercben adandó meg (0…86400).

## Telítés és kondenzátum

M_sat=V*AH_sat(T). Ha a dinamika telítést érne el, a modell analitikusan kiszámítja az elérés időpontját, majd telített szakaszon a nettó bejövő többletet kondenzátumként számolja. Nem pusztán levágja az RH-t 100%-nál: a leválasztott víz nem tűnik el a könyvelésből. Emiatt egy 3600 másodperces és 3600 darab egy másodperces állandó határfeltételű lépés kondenzátuma is megegyezik numerikus tolerancián belül.

A `setTemperatureC()` a vízgőztömeget megőrzi, amíg nem érjük el a telítést. Telítésen túli hűtéskor a többlet kondenzátumba kerül. A kondenzátum elvezetettnek tekintett, nem párolog vissza automatikusan. Ez explicit modellfeltétel; nedves falfelületből történő visszapárolgásra a G forrás csak egyszerűsített közelítés.

Minden lépésben ellenőrizhető:

```
M_current + water_out + condensate = M_initial + generated + water_in
```

Minden tag grammban értendő, a be/ki és forrástagok kumulatívak. Melegítés fix térfogatban, légcsere és kondenzáció nélkül megtartja M-et és AH-t, miközben RH változik.

## Határok és tesztek

Nincs közvetlen RH állapotváltozó. Nincs épület-hőmodell, nyomásdinamika, szivárgás, kondenzációs látens hő vagy mért felületi hőmérséklet. A térfogatra vonatkozó egyszerűsítés nyomáskiegyenlítődő, kis vízgőz-tömegáramú pincei közelítés; nem teljes szárazlevegő- és energiaegyenleg. Ebből nem következik a valós felületek kondenzációmentessége.

A `test_mass_balance` ellenőrzi a zárt levegő melegítését/hűtését, az ismert g/h forrást, az analitikus légcserét, a kondenzátum megőrzését, az időlépésfüggetlenséget és 10 000 determinisztikus változó állapot mérlegzárását. A hibás paramétereket és kültéri határállapotot a modell visszautasítja.
