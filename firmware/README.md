# PHASE 5–8 az új specifikáció szerint

Az ESP32 Arduino implementáció helye: sensors/, display/, relay/, dehumidifier/, storage/, network/. Ezek a következő fázisok; a jelenlegi Windows build nem tesz hardverhívást és nem tartalmaz kész LVGL/NVS/REST implementációt. A firmware az új DryingController-t használja majd. A dehumidifier mappába kerül a tényleges IDehumidifierMonitor adapter. GPIO és relé/kontaktor-kialakítás: NEEDS HARDWARE VERIFICATION / NEEDS ELECTRICAL DESIGN / VERIFICATION.
