# Végleges rétegezés

Adatút: ISensor → ClimateReading → VentilationController → ControllerOutput → IRelay. Az IClock monoton, 64 bites milliszekundumot szolgáltat. Az UI és HTTP ugyanazt a konfigurációvalidálást és módváltást használja majd; a core sem relét, sem hálózatot, sem LVGL-t nem ismer.

Az IStorage a verziózott Configuration betöltésének/mentésének határa. A későbbi NVS adapter mezőnként szerializáljon, ne nyers C++ struktúramemóriát írjon; ellenőrizze a verziót, teljes rekordot és isValid()-ot. Mentés csak kifejezett felhasználói kérésre. A HAL nem tartalmaz implementációt a mostani két fázisban.

A CMake a core és host tesztek elsődleges buildje. PlatformIO külön native környezetet kapott; az ESP32 target csak a hardverellenőrzés és a 4. fázis után kerül be, így nem sugallunk hamis készültséget egy általános ESP32 boardazonosítóval. C++17 a kompatibilis közös nyelvi szint. A test/ név a PlatformIO konvencióját követi.

Később egyetlen firmware ciklus olvassa a szenzorokat, értékeli a core-t és írja a relét; a kijelző és HTTP csak parancsokat ad át. A szenzorhibás mintát nem szabad új timestamp-pel friss sikeres mérésnek feltüntetni. A millis() 32 bites átfordulását a HAL-nak kell 64 bitre bővítenie, vagy ESP monotonic 64 bites időforrást használni.

A 24 órás történet RAM ring buffer lesz 1440 percenkénti rekorddal; hibás mintákat külön érvényesség jelöl. A páramentesítő később önálló actuator policy-t kap; a jelenlegi dehumidificationSuggested kizárólag javaslat, nem elektromos kimenet.
