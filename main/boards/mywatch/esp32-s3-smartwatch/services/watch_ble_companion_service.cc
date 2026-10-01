#include "watch_ble_companion_service.h"

#include "watch_phone_service.h"

#include <esp_log.h>
#include <host/ble_hs.h>
#include <host/ble_uuid.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>
#include <host/ble_store.h>
#include <store/config/ble_store_config.h>

#include <cstring>
#include <cstdio>

#define TAG "WatchBle"

extern "C" void ble_store_config_init(void);

namespace {
constexpr char kDeviceName[] = "MyWatch";
constexpr uint16_t kNoConnection = BLE_HS_CONN_HANDLE_NONE;
constexpr ble_uuid128_t kServiceUuid = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e);
constexpr ble_uuid128_t kRxUuid = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e);
constexpr ble_uuid128_t kTxUuid = BLE_UUID128_INIT(
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e);

WatchBleCompanionService* g_service = nullptr;
uint8_t g_address_type = 0;
uint8_t g_rx_argument = 1;
uint8_t g_tx_argument = 2;
uint16_t g_tx_value_handle = 0;

ble_gatt_chr_def kCharacteristics[] = {
    {.uuid = &kRxUuid.u,
     .access_cb = WatchBleCompanionService::CharacteristicAccess,
     .arg = &g_rx_argument,
     .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_ENC},
    {.uuid = &kTxUuid.u,
     .access_cb = WatchBleCompanionService::CharacteristicAccess,
     .arg = &g_tx_argument,
     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
     .val_handle = &g_tx_value_handle},
    {0},
};

const ble_gatt_svc_def kServices[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &kServiceUuid.u,
     .characteristics = kCharacteristics},
    {0},
};
}  // namespace

bool WatchBleCompanionService::Start() {
    if (started_) return true;
    if (g_service != nullptr) return false;
    g_service = this;

    const esp_err_t error = nimble_port_init();
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "NimBLE init failed: %s", esp_err_to_name(error));
        g_service = nullptr;
        return false;
    }

    ble_hs_cfg.reset_cb = [](int reason) { ESP_LOGE(TAG, "Host reset: %d", reason); };
    ble_hs_cfg.sync_cb = OnSync;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_store_config_init();
    int result = ble_gatts_count_cfg(kServices);
    if (result == 0) result = ble_gatts_add_svcs(kServices);
    if (result == 0) result = ble_svc_gap_device_name_set(kDeviceName);
    if (result != 0) {
        ESP_LOGE(TAG, "GATT setup failed: %d", result);
        nimble_port_deinit();
        g_service = nullptr;
        return false;
    }

    nimble_port_freertos_init(HostTask);
    started_ = true;
    ESP_LOGI(TAG, "Companion BLE service starting");
    return true;
}

void WatchBleCompanionService::HostTask(void* argument) {
    (void)argument;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void WatchBleCompanionService::OnSync() {
    if (ble_hs_id_infer_auto(0, &g_address_type) != 0) {
        ESP_LOGE(TAG, "Could not infer BLE address type");
        return;
    }
    if (g_service != nullptr) g_service->StartAdvertising();
}

void WatchBleCompanionService::StartAdvertising() {
    ble_hs_adv_fields fields{};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = reinterpret_cast<const uint8_t*>(kDeviceName);
    fields.name_len = sizeof(kDeviceName) - 1;
    fields.name_is_complete = 1;
    fields.uuids128 = const_cast<ble_uuid128_t*>(&kServiceUuid);
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;
    int result = ble_gap_adv_set_fields(&fields);
    if (result != 0) {
        ESP_LOGE(TAG, "Advertisement fields failed: %d", result);
        return;
    }
    ble_gap_adv_params parameters{};
    parameters.conn_mode = BLE_GAP_CONN_MODE_UND;
    parameters.disc_mode = BLE_GAP_DISC_MODE_GEN;
    result = ble_gap_adv_start(g_address_type, nullptr, BLE_HS_FOREVER, &parameters, GapEvent,
                               g_service);
    if (result != 0) ESP_LOGE(TAG, "Advertising start failed: %d", result);
    else ESP_LOGI(TAG, "Advertising as %s", kDeviceName);
}

int WatchBleCompanionService::GapEvent(ble_gap_event* event, void* context) {
    auto* service = static_cast<WatchBleCompanionService*>(context);
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                service->connection_handle_ = event->connect.conn_handle;
                service->phone_.SetConnected(true);
                ESP_LOGI(TAG, "Phone connected; starting bonded encryption");
                const int result = ble_gap_security_initiate(service->connection_handle_);
                if (result != 0) ESP_LOGW(TAG, "Security initiation returned %d", result);
            } else {
                service->StartAdvertising();
            }
            return 0;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Phone disconnected: reason=%d", event->disconnect.reason);
            service->phone_.SetConnected(false);
            service->connection_handle_ = kNoConnection;
            service->tx_notifications_enabled_ = false;
            service->frame_size_ = 0;
            service->dropping_frame_ = false;
            service->StartAdvertising();
            return 0;
        case BLE_GAP_EVENT_SUBSCRIBE:
            service->tx_notifications_enabled_ = event->subscribe.cur_notify != 0;
            return 0;
        case BLE_GAP_EVENT_ENC_CHANGE:
            if (event->enc_change.status == 0) ESP_LOGI(TAG, "BLE link encrypted and bonded");
            else ESP_LOGW(TAG, "BLE encryption failed: %d", event->enc_change.status);
            return 0;
        case BLE_GAP_EVENT_REPEAT_PAIRING: {
            ble_gap_conn_desc description{};
            if (ble_gap_conn_find(event->repeat_pairing.conn_handle, &description) == 0)
                ble_store_util_delete_peer(&description.peer_id_addr);
            return BLE_GAP_REPEAT_PAIRING_RETRY;
        }
        default:
            return 0;
    }
}

int WatchBleCompanionService::CharacteristicAccess(uint16_t connection, uint16_t attribute,
                                                    ble_gatt_access_ctxt* context, void* argument) {
    (void)connection;
    (void)attribute;
    const uint8_t type = argument == nullptr ? 0 : *static_cast<uint8_t*>(argument);
    if (g_service == nullptr) return BLE_ATT_ERR_UNLIKELY;
    if (context->op == BLE_GATT_ACCESS_OP_WRITE_CHR && type == 1) {
        std::array<uint8_t, 768> fragment{};
        uint16_t copied = 0;
        const int result = ble_hs_mbuf_to_flat(context->om, fragment.data(), fragment.size(), &copied);
        if (result != 0) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        g_service->Consume(fragment.data(), copied);
        return 0;
    }
    if (context->op == BLE_GATT_ACCESS_OP_READ_CHR && type == 2)
        return g_service->ReadStatus(context);
    return BLE_ATT_ERR_UNLIKELY;
}

void WatchBleCompanionService::Consume(const uint8_t* data, size_t length) {
    for (size_t index = 0; index < length; ++index) {
        const char byte = static_cast<char>(data[index]);
        if (dropping_frame_) {
            if (byte == '\n') dropping_frame_ = false;
            continue;
        }
        if (byte == '\n') {
            size_t size = frame_size_;
            if (size > 0 && frame_[size - 1] == '\r') --size;
            const bool accepted = size > 0 && phone_.HandleMessage(frame_.data(), size);
            if (size == 0) phone_.RejectMessage();
            ESP_LOGI(TAG, "Companion JSON line %s (%u bytes)", accepted ? "accepted" : "rejected",
                     static_cast<unsigned>(size));
            Acknowledge(accepted);
            frame_size_ = 0;
            continue;
        }
        if (frame_size_ >= frame_.size() - 1) {
            frame_size_ = 0;
            dropping_frame_ = true;
            phone_.RejectMessage();
            Acknowledge(false);
            continue;
        }
        frame_[frame_size_++] = byte;
    }
}

void WatchBleCompanionService::Acknowledge(bool accepted) {
    if (!tx_notifications_enabled_ || connection_handle_ == kNoConnection || g_tx_value_handle == 0)
        return;
    const char* response = accepted ? "OK\n" : "ERR\n";
    os_mbuf* packet = ble_hs_mbuf_from_flat(response, strlen(response));
    if (packet == nullptr || ble_gatts_notify_custom(connection_handle_, g_tx_value_handle, packet) != 0)
        ESP_LOGW(TAG, "Could not send BLE acknowledgement");
}

int WatchBleCompanionService::ReadStatus(ble_gatt_access_ctxt* context) {
    const WatchPhoneSnapshot snapshot = phone_.GetSnapshot();
    char status[80];
    const int length = snprintf(status, sizeof(status), "connected=%u;received=%lu;rejected=%lu",
                                snapshot.connected ? 1U : 0U,
                                static_cast<unsigned long>(snapshot.received_messages),
                                static_cast<unsigned long>(snapshot.rejected_messages));
    return os_mbuf_append(context->om, status, length) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}
