# PHASE 5–8 az új specifikáció szerint

0.6 hardvercél: ESP32-C3 + SH1106 OLED + két gomb, távoli C3/SHT30 RS485 csomópont, külön felügyelt úszójel. Terv: ../docs/oled_hardware_plan.md. Az ITankSensor adapter és a valódi RS485/SH1106 implementáció még hátra van; a korábbi LVGL terv történeti opció.

Az ESP32 Arduino implementáció helye: sensors/, display/, relay/, dehumidifier/, storage/, network/. Ezek a következő fázisok; a jelenlegi Windows build nem tesz hardverhívást és nem tartalmaz kész LVGL/NVS/REST implementációt. A firmware az új DryingController-t használja majd. A dehumidifier mappába kerül a tényleges IDehumidifierMonitor adapter. GPIO és relé/kontaktor-kialakítás: NEEDS HARDWARE VERIFICATION / NEEDS ELECTRICAL DESIGN / VERIFICATION.
