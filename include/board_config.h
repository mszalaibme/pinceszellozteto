#pragma once
// NEEDS HARDWARE VERIFICATION: no GPIO assignment inferred from product name.
namespace ventilation::board {
enum class RelayPolarity { Unknown, ActiveHigh, ActiveLow };
inline constexpr bool verified = false;
inline constexpr int tftSclk=-1, tftMosi=-1, tftMiso=-1, tftCs=-1, tftDc=-1, tftReset=-1;
inline constexpr int touchSda=-1, touchScl=-1, touchIrq=-1, touchReset=-1;
inline constexpr int sdSclk=-1, sdMosi=-1, sdMiso=-1, sdCs=-1;
inline constexpr int backlight=-1, sensorSda=-1, sensorScl=-1, relay=-1;
inline constexpr int dehumidifierRelay=-1, dehumidifierMonitor=-1;
inline constexpr RelayPolarity relayPolarity=RelayPolarity::Unknown;
inline constexpr RelayPolarity dehumidifierRelayPolarity=RelayPolarity::Unknown;
}
