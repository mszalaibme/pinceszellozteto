> Történeti 0.6 OLED-változat. Aktuális terv: [CYD 2,8 hüvelyk](cyd28_hardware_plan.md).

# OLED és úszó validáció 0.6

Before fix: regression test_tank_interlock failed, old FullTank was only NoPowerDraw.
17: FAIL: full tank leaves dehumidifier commanded ON during minimum ON
FAIL: full tank leaves dehumidifier commanded ON during minimum ON
	 17 - test_tank_interlock (Failed)

After fix: 20/20 CTest groups passed, including immediate tank inhibition, invalid/stale/future input, reboot, manual override, minimum OFF and continued ventilation.

A batch külön, ugyanazzal az opcionális tankbemenetet nem használó régi futtatóval számol; az új úszót az interaktív session és a core-regressziók validálják.
