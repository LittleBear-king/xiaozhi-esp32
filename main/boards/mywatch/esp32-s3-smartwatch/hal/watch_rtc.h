#ifndef _MYWATCH_WATCH_RTC_H_
#define _MYWATCH_WATCH_RTC_H_

#include <driver/i2c_master.h>

#include <cstdint>
#include <ctime>

class WatchRtc final {
public:
    WatchRtc(i2c_master_bus_handle_t bus, uint8_t address);
    ~WatchRtc();

    bool Initialize();
    bool Read(struct tm& local_time);
    bool Write(const struct tm& local_time);
    bool IsAvailable() const { return device_ != nullptr; }

private:
    static uint8_t ToBcd(int value);
    static int FromBcd(uint8_t value);

    i2c_master_bus_handle_t bus_ = nullptr;
    i2c_master_dev_handle_t device_ = nullptr;
    uint8_t address_ = 0;
};

#endif  // _MYWATCH_WATCH_RTC_H_
