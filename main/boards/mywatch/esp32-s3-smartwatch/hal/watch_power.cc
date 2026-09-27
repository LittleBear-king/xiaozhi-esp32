#include "watch_power.h"

WatchPower::WatchPower(i2c_master_bus_handle_t i2c_bus, uint8_t address)
    : Axp2101(i2c_bus, address) {
    WriteReg(0x22, 0b110);  // Enable long-press power-off.
    WriteReg(0x27, 0x10);   // Hold the power button for four seconds to shut down.

    WriteReg(0x80, 0x01);  // Keep DC1 enabled and disable the other DC rails.
    WriteReg(0x90, 0x00);
    WriteReg(0x91, 0x00);
    WriteReg(0x82, (3300 - 1500) / 100);  // DC1: 3.3 V.

    WriteReg(0x92, (3300 - 500) / 100);  // ALDO1: 3.3 V.
    WriteReg(0x93, (3300 - 500) / 100);  // ALDO2: 3.3 V.
    WriteReg(0x90, 0x03);                // Enable the microphone rails.

    WriteReg(0x64, 0x02);  // Charge voltage: 4.1 V.
    WriteReg(0x61, 0x02);  // Precharge current: 50 mA.
    WriteReg(0x62, 0x0A);  // Charge current: 400 mA.
    WriteReg(0x63, 0x01);  // Termination current: 25 mA.
}
