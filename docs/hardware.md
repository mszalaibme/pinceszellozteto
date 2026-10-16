# Board inventory — NEEDS HARDWARE VERIFICATION

ESP32-3248S035C-CYD pontos revision és gyártói kapcsolási rajz nincs megadva. A típusnév alapján nem rendelünk GPIO-t.

| Funkció | Pin / típus jelenleg | Fizikai ellenőrzés |
|---|---|---|
| TFT | Ismeretlen | vezérlő IC, busz, összes adat/vezérlőpin |
| Kapacitív touch | Ismeretlen | IC, I2C cím, SDA/SCL, IRQ, RESET |
| SD | Ismeretlen | busz, CS és megosztott pinek |
| Backlight | Ismeretlen | GPIO, polaritás, driver |
| Szenzor I2C | Ismeretlen | szabad SDA/SCL, felhúzók, kábelhossz |
| Szabad GPIO | Nincs igazolt lista | kapcsolási rajz és folytonosságmérés |
| Relé | Ismeretlen | pin, 3.3 V kompatibilitás, active HIGH/LOW |
| Páramentesítő kontaktorvezérlés | Ismeretlen | külön GPIO, polaritás, tekercs/driver |
| Páramentesítő monitor | Ismeretlen | mérési mód, galvanikus leválasztás, kommunikáció |

Az include/board_config.h összes pinje -1, a polaritás Unknown. A még nem kész firmware nem használhatja ezeket ellenőrzés nélkül. ESP32 boot strap, flash, bemenet-only és bekapcsolási tranziensek vizsgálata kötelező a konkrét SoC és board alapján.

Két SHT30/31 alapértelmezett címe ütközhet: ellenőrizendő címválasztási lehetőség (tipikusan 0x44/0x45, de a konkrét modul dokumentációja dönt), külön busz vagy I2C multiplexer szükséges lehet. MCP23017 GPIO expander önmagában nem I2C címmultiplexer. Kültéri hosszú I2C kábel megbízhatóságát külön validálni kell. Szenzorhibát az adapternek valid=false jelzéssel kell továbbadnia.
