// NimBLE Server
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "sensor.h"
#include "ble.h"

// The infinite task
void host_task(void *param)
{
    nimble_port_run();
}

void app_main()
{
    printf("Got here\n");
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    printf("Got here\n");
    esp_nimble_hci_init();                     // 2 - Initialize ESP controller
    printf("Got here\n");
    nimble_port_init();                        // 3 - Initialize the host stack
    printf("Got here\n");
    ble_svc_gap_device_name_set("BLE-Server"); // 4 - Initialize NimBLE configuration - server name
    printf("Got here\n");
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    printf("Got here\n");
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    printf("Got here\n");
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    printf("Got here\n");
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    printf("Got here\n");
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    printf("Got here\n");
    nimble_port_freertos_init(host_task);      // 6 - Run the thread
    printf("Got here\n");
    xTaskCreate(push_button_task, "push_button_task", 2048, NULL, 5, NULL);
    printf("Got here\n");
    xTaskCreate(sensor_task, "sensor_task", 2048, NULL, 5, NULL);
    printf("Got here\n");
    xTaskCreate(event_start_task, "event_start_task", 2048, NULL, 5, NULL);
    printf("Got here\n");
}