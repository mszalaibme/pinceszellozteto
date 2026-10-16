# Architektúra — v2 specifikáció

A core kizárólag C++17 standard típusokat használ. Az adatút: ISensor + IDehumidifierMonitor + IClock → DryingController → két logikai kimenet → két külön IRelay/kontaktor adapter. A controller nem tud GPIO-ról, LVGL-ről, Wi-Fi-ről, készülékmodellről vagy 230 V-ról.

A Configuration egyetlen validált paraméterobjektum. Az új mezők nevei a v2 specifikációt követik. A mode és a konfiguráció az UI/HTTP felől ugyanazon core metódusokon keresztül változik. A későbbi firmware egyetlen vezérlőciklusban kezeli a méréseket és kimeneteket; UI/HTTP nem írhat közvetlenül relét.

A `DehumidifierReading` a típusos feedback mellett opcionális teljesítményt és timestampet ad. A fizikai tartályszint nem része a controller bemenetének. Így a szimulátor valóban csak a monitorból felismerhető NO_POWER_DRAW állapotot diagnosztizálhatja, nem olvassa titokban a virtuális tankFull flaget.

A simulator könyvtár tartalmazza a BasementModel, DehumidifierModel és SimulationRunner osztályokat. A szellőztetés tömegmérlege a BasementModel része, a kültéri profil a SimulationRunner forgatókönyvi határfeltétele: ezért nincs üres VentilationModel/OutdoorModel wrapper. A `scenarios()` a 12 explicit forgatókönyv egyetlen definíciója; a tesztek és CLI ugyanazt használják. A CLI CSV-t és beágyazott adatokkal önálló HTML-riportot készít.

A korábbi VentilationController a validáció történeti regresszióihoz maradt meg. Az új termékkód a DryingController-t használja; a régi RH-szabályt nem vegyítjük az új runtime viselkedéssel.

A HAL interfészek firmware adapterei, az NVS, LVGL, REST és RAM history a következő fázisok. Az NVS majd mezőnként, verziózott rekordként tároljon, ne nyers struktúramemóriát írjon. Sérült/hiányos konfiguráció esetén ellenőrzött default és mindkét kimenet KI. A 32 bites millis átfordulását a HAL-nak kell bővítenie; az időforrás 64 bites monoton milliszekundum.

A natív CMake build készíti a core-t, szimulációt, EXE-t és teszteket. A Windows-csomag statikus MSVC runtime-ot használ. A CI Windows Release környezetben fut és kész csomagot ad artifactként.
