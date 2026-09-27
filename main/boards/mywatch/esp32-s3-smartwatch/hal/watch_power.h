#ifndef _MYWATCH_WATCH_POWER_H_
#define _MYWATCH_WATCH_POWER_H_

#include "axp2101.h"

class WatchPower final : public Axp2101 {
public:
    WatchPower(i2c_master_bus_handle_t i2c_bus, uint8_t address);
};

#endif  // _MYWATCH_WATCH_POWER_H_
