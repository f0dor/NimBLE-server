#ifndef BLE_H
#define BLE_H

#include <stdio.h>
#include <inttypes.h>
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sensor.h"

#define MY_SERVICE_UUID             BLE_UUID16_DECLARE(0x1405)
#define MY_READ_CHR_UUID            BLE_UUID16_DECLARE(0x0508)
#define MY_WRITE_CHR_UUID           BLE_UUID16_DECLARE(0x0512)
#define MY_RTT_CHR_UUID             BLE_UUID16_DECLARE(0x0516)

extern char *TAG;
extern uint8_t ble_addr_type;
extern uint16_t notify_handle;
extern uint16_t rtt_notify_handle;
extern uint16_t connection_handle;
extern float timestamp_value;
extern bool advertising_in_progress;
extern bool is_connected;
extern bool is_notified;
extern bool started_state;
extern const struct ble_gatt_svc_def gatt_svcs[];


void print_connection_parameters(uint16_t conn_handle);

int timestamp_read(uint16_t conn_handle, uint16_t attr_handle,
                                                struct ble_gatt_access_ctxt *ctxt, void *arg);
int timestamp_write(uint16_t conn_handle, uint16_t attr_handle,
                                                struct ble_gatt_access_ctxt *ctxt, void *arg);
int indicate(uint16_t conn_handle, uint16_t attr_handle,
                                                struct ble_gatt_access_ctxt *ctxt, void *arg);
int notify(uint16_t conn_handle, uint64_t data);

int notify_rtt(uint16_t conn_handle, uint64_t data);

int ble_gap_event(struct ble_gap_event *event, void *arg);

void ble_app_advertise(void);

void ble_app_on_sync(void);

#endif /* BLE_H */