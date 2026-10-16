# PC szimuláció — PHASE 3 részben kész

A BasementModel tiszta C++17 könyvtár, ESP32 nélkül fordul. A víztömegmérlegmodell kész és unit tesztelt; a forgatókönyves CLI, CSV export és plot még a 3. fázis további része.

Állapotváltozó a vízgőz tömege grammban. A relatív páratartalmat soha nem integráljuk vagy interpoláljuk közvetlenül. Részletek: [moisture_model.md](../docs/moisture_model.md).

A kültéri mérés a fizikai határállapot. A szenzorhiba injektálását a vezérlőnek adott mérésben kell elvégezni, nem a fizikai levegő állapotát NaN-ra állítani. A hőmérséklet explicit előírt határfeltétel; a modell nem állítja, hogy hőtároló falakat, hővezetést vagy nyomásdinamikát szimulál.
