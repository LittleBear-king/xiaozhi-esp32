#ifndef _MYWATCH_WATCH_MOTION_H_
#define _MYWATCH_WATCH_MOTION_H_

#include <driver/i2c_master.h>

#include <cstdint>
#include <memory>

struct WatchAcceleration {
    float x_mg = 0;
    float y_mg = 0;
    float z_mg = 0;
};

class WatchMotion final {
public:
    WatchMotion(i2c_master_bus_handle_t i2c_bus, uint8_t address);
    ~WatchMotion();

    bool Initialize();
    bool ReadAcceleration(WatchAcceleration& acceleration);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

#endif  // _MYWATCH_WATCH_MOTION_H_
