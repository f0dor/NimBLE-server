#include "ble.h"
#include "sensor.h"

char *TAG = "BLE-Server";
uint8_t ble_addr_type;
uint16_t notify_handle;
uint16_t rtt_notify_handle;
uint16_t connection_handle = 0;
float timestamp_value = 0;

bool advertising_in_progress = false;
bool is_connected = false;
bool is_notified = false;


const struct ble_gatt_svc_def gatt_svcs[] = {       // UUID - Universaly Unique Identifier
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = MY_SERVICE_UUID,                       // UUID koji definira servis
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = MY_READ_CHR_UUID,                 // UUID koji definira karakteristiku čitanja
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = timestamp_read},
         {.uuid = MY_WRITE_CHR_UUID,                // UUID koji definira karakteristiku pisanja
          .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
          .access_cb = timestamp_write,
          .val_handle = &notify_handle},
          {.uuid = MY_RTT_CHR_UUID,
          .flags = BLE_GATT_CHR_F_NOTIFY,
          .access_cb = NULL,
          .val_handle = &rtt_notify_handle},
         {0}}},
    {0}};


void print_connection_parameters(uint16_t conn_handle)
{
    struct ble_gap_conn_desc desc;
    int rc;

    // Find the connection entry based on the connection handle
    rc = ble_gap_conn_find(conn_handle, &desc);
    if (rc != 0)
    {
        printf("Error finding connection: %d\n", rc);
    }

    // Print the connection parameters
    printf("Connection Parameters:\n");
    printf("\tConnection Handle: %d\n", conn_handle);
    printf("\tOur ID Address: ");
    for (int i = 0; i < sizeof(ble_addr_t); i++) {
            printf("%02x ", desc.our_id_addr.val[i]);
        }
        printf("\n");
    printf("\tConnection Interval: %u units\n", desc.conn_itvl);
    printf("\tSlave Latency: %u\n", desc.conn_latency);
    printf("\tSupervision Timeout: %u units\n", desc.supervision_timeout);
}

int timestamp_read(uint16_t conn_handle, uint16_t attr_handle, 
    struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = os_mbuf_append(ctxt->om, &timestamp_value, sizeof(float));

    if (rc != 0)
    {
        printf("Error appending data to mbuf: %d\n", rc);
        return BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return 0;
}

int timestamp_write(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    is_notified = false;
    eventStarted = false;
    printf("Data from the client: ");
    for (int i = 0; i < ctxt->om->om_len; i++) {
        printf("%u ", ctxt->om->om_data[i]);
    }
    printf("\n");

    if (ctxt->om->om_len == sizeof(float)) {
        float received_value;
        memcpy(&received_value, ctxt->om->om_data, sizeof(float));
        printf("It took %.3f seconds!\n", received_value);
    }
    return 0;
}

int notify(uint16_t conn_handle, uint64_t data)
{
    struct os_mbuf *om;

    om = ble_hs_mbuf_from_flat(&data, sizeof(&data));
    ESP_LOGI(TAG, "Notifying conn=%d", conn_handle);
    int rc = ble_gattc_notify_custom(conn_handle, notify_handle, om);
    if (rc != 0) {
        ESP_LOGE(TAG, "error notifying; rc=%d", rc);
        return rc;
    }
    return 0;
}

int notify_rtt(uint16_t conn_handle, uint64_t data) {
    struct os_mbuf *om;
    om = ble_hs_mbuf_from_flat(&data, sizeof(data));
    ESP_LOGI(TAG, "Notifying RTT conn=%d", conn_handle);
    int rc = ble_gattc_notify_custom(conn_handle, rtt_notify_handle, om);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error notifying RTT; rc=%d", rc);
        return rc;
    }
    return 0;
}

// BLE event handling
int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", 
        event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status == 0)
        {
            printf("Connection established with client\n");
            is_connected = true;
            connection_handle = event->connect.conn_handle;
            vTaskDelay(pdMS_TO_TICKS(500));
            print_connection_parameters(event->connect.conn_handle);
            ble_gap_adv_stop();
            advertising_in_progress = false;
        }
        else
        {
            printf("Connection failed with status: %d\n", event->connect.status);
            ble_app_advertise();
        }
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT ADVERTISEMENT COMPLETE");
        ble_app_advertise();
        advertising_in_progress = false;
        is_connected = false;
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE DEVICE DISCONNECTED");
        is_connected = false;
        is_notified = false;
        advertising_in_progress = false;
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}



// Define the BLE connection

void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;

    // Include the list of service UUIDs in the advertisement
    fields.uuids16 = MY_SERVICE_UUID;
    fields.num_uuids16 = 1;

    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
    advertising_in_progress = true;
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    // ble_app_advertise();                     // Define the BLE connection
}