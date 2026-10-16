# Fejlesztési útmutató

A fejlesztés alapága `dev`. Kisebb változtatásokhoz ebből induló külön ág javasolt; integrálás előtt minden teszt fusson le.

```sh
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure -V
```

A core kizárólag standard C++17-re támaszkodhat. Arduino, ESP32, LVGL, Wi-Fi és szenzorkönyvtárak a firmware adapterrétegbe kerülnek. A szimulátor és a tesztek ugyanazt a core-t használják.

Minden fejlesztési fázis végén build és teszt szükséges; sikertelen ellenőrzéssel a következő fázis nem indulhat. Az adott fázis funkcióiban nem maradhat helykitöltő. A későbbi fázisok állapotát a README táblázatában jelezzük.

Viselkedésváltozáshoz releváns teszt és CHANGELOG bejegyzés kell. Ismeretlen hardveradatot ne becsülj: `NEEDS HARDWARE VERIFICATION` jelölés és board_config.h elkülönítés szükséges. A hálózati feszültségű terhelés nem része a fejlesztői tesztkörnyezetnek.

Licenc még nincs kiválasztva. Nyilvános hozzáférés önmagában nem jelent szabad felhasználási licencet; a jogosult döntése nélkül nem rendelünk licencet a projekthez.
