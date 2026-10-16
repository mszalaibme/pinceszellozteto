# Aktuális 2,8 hüvelykes ESP32 hardverterv (0.7)

## Egy integrált központ, egy távoli mérő

Központ: **ESP32-2432S028R-CYD**, HESTORE 100.503.63, 2,8 hüvelyk, 240×320 TFT, ILI9341, rezisztív érintés. Ez egyszerre a fő vezérlő és a kijelző: külön központi ESP32-C3, OLED és két nyomógomb nem kell. A ventilátor és páramentesítő parancsa, az időzítők, a gomb nélküli kézi/automata kezelés a központ feladata.

Pince: **1 ESP32-C3-SUPERMINI**, kijelző nélkül, a pincei SHT30 mellett. A szenzor rövid helyi I2C vezetékkel csatlakozik; a központig az 5–6 m kapcsolat RS485, egy MAX3485 modullal mindkét végén. Ez a második ESP32 a mérés átalakításához és továbbításához kell, nem a kijelző meghajtásához. Más protokollú kész szenzorral második MCU nélkül is lehetne tervezni, de az más beszerzés lenne. A jelenlegi tervben összesen két ESP32 van, közülük csak az egyik kijelzős.

Kültér: második SHT30-M, a felhasználó által jelzett tető alatt, direkt víztől védve, központ közelében. Szellőző árnyékolás, páralecsapódás és nyitott panel védelme szükséges. Nem légmentes dobozba kerül; a kész, védett DF-SEN0385 szonda kikerült. A két SHT30 külön MCU I2C buszán van, címük nem ütközik.

## Mi marad ellenőrizendő?

Az integrált kijelző valóban egyszerűsít, de nem jelent automatikusan forrasztásmentes rendszert. A pincei C3 tüskesora, szerelőpanelek, bias/lezáró ellenállások, relétáp és a felügyelt úszójel még szerelést igényelnek. Precíz pákát igénylő SMD beültetést nem tervezünk, de ez jelenleg nem tisztán csavarozható/csatlakoztatható készlet. A végleges forrasztásmentes adapterkiválasztás nincs kész; ezt vásárlás előtt rendezni kell.

A CYD sok lábát a TFT, touch, SD, LED és egyéb áramkörök használják. A teljes GPIO-tervet a tényleges revízió alapján kell ellenőrizni. A C3-hoz korábban gondolt szabad ADC-pin nem másolható át a CYD-re: a felügyelt úszóbemenetnek igazolt ADC vagy külső ADC adapter kell; ez a kiválasztás még nyitott. Digitális MCP23017 önmagában nem méri a végellenállást. A szoftveres invalid/full védelmek teszteltek, a fizikai adapter még nem kész.

A kosár két 4 pólusú, 1,25 mm-es szerelt kábelt tartalmaz csatlakozójelöltként. A pitch önmagában nem kompatibilitási garancia: a CYD konkrét csatlakozócsaládját, kulcsolását és pin-sorrendjét ellenőrizni kell. USB-C kábel és 5 V USB-A táp marad. A pincei csomópont tápellátása, RS485 lezárás/bias és a relék külön 3,3 V-os táplálása a korábbi terv elveit követi; nem kész hálózati bekötési útmutató.

A P31 úszó megmarad. Tele/hibás/elavult jel letiltja a páramentesítőt, minimum BE és kézi parancs ellenére is. Árammérő nincs, tehát a bekapcsolási parancs továbbra sem igazolt kompresszorüzem. Az úszó valódi mechanikai elférése, a kivehető tartály oldható csatlakozója és felügyelt jeladaptere még ellenőrizendő. A készülék saját tartályvédelme megmarad.

A közös C++ mag és a Windows UI kész; ESP32 firmware, CYD érintéskalibráció, flash/NVS, szenzor/RS485/GPIO adapter és a hálózati hardvervalidáció nem készült el. A [korábbi OLED terv](oled_hardware_plan.md) csak történeti referencia.

Termékforrás: https://www.hestore.hu/prod_10050363.html ; forgalmazói adatlap: https://www.hestore.hu/prod_getfile.php?id=21876 .
