#ifndef _MYWATCH_WATCH_BLE_COMPANION_SERVICE_H_
#define _MYWATCH_WATCH_BLE_COMPANION_SERVICE_H_

#include <array>
#include <cstddef>
#include <cstdint>

class WatchPhoneService;

class WatchBleCompanionService final {
public:
    explicit WatchBleCompanionService(WatchPhoneService& phone) : phone_(phone) {}
    bool Start();

    static int GapEvent(struct ble_gap_event* event, void* context);
    static int CharacteristicAccess(uint16_t connection, uint16_t attribute,
                                    struct ble_gatt_access_ctxt* context, void* argument);

private:
    static void OnSync();
    static void HostTask(void* argument);
    void StartAdvertising();
    void Consume(const uint8_t* data, size_t length);
    void Acknowledge(bool accepted);
    int ReadStatus(struct ble_gatt_access_ctxt* context);

    WatchPhoneService& phone_;
    std::array<char, 769> frame_{};
    size_t frame_size_ = 0;
    bool dropping_frame_ = false;
    uint16_t connection_handle_ = 0xffff;
    bool tx_notifications_enabled_ = false;
    bool started_ = false;
};

#endif  // _MYWATCH_WATCH_BLE_COMPANION_SERVICE_H_
