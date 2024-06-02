#include "sensor.h"
#include "ble.h"

const char *PBTN = "Push_Button_Task";
const char *SNSR = "Sensor_Task";

uint64_t echoStartTime = 0;
uint64_t echoEndTime = 0;
bool echoPulseStarted = false;
bool eventStarted = false;

//______________________________________________________//

gptimer_handle_t gptimer = NULL;
gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT,
    .direction = GPTIMER_COUNT_UP,
    .resolution_hz = 1000000
};


//__________________GPIO configuration__________________//

gpio_config_t io_conf_ble_start = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = (1ULL << BLE_START_PIN),
    .pull_down_en = 0,
    .pull_up_en = 1
};

gpio_config_t io_conf_sensor = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = (1ULL << SENSOR_TRIGGER_PIN),
    .pull_down_en = 0,
    .pull_up_en = 0
};

gpio_config_t io_conf_echo = {
    .intr_type = GPIO_INTR_ANYEDGE,
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = (1ULL << ECHO_PIN),
    .pull_down_en = 1,
    .pull_up_en = 0
};

gpio_config_t io_conf_event_start = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_INPUT,
    .pin_bit_mask = (1ULL << EVENT_START_PIN),
    .pull_down_en = 0,
    .pull_up_en = 1
};

//______________________________________________________//

void IRAM_ATTR echoPinHandler(void *arg) {
    if (gpio_get_level(ECHO_PIN) == 1) {
        gptimer_start(gptimer);
        echoPulseStarted = true;
    } else {
        gptimer_get_raw_count(gptimer, &echoEndTime);
        gptimer_stop(gptimer);
        echoPulseStarted = false;
    }
}


void sensor_task(void *parameters) {
    float distance = 0.0;
    float ticks_duration = 0.0;
    uint32_t ticks_num = 0;
    gpio_config(&io_conf_sensor);
    gpio_set_intr_type(ECHO_PIN, GPIO_INTR_ANYEDGE);
    
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gpio_install_isr_service(0);
    gptimer_enable(gptimer);
    gpio_isr_handler_add(ECHO_PIN, echoPinHandler, NULL);

    printf("Started sensor task!\n");

    while(true) {
        if(eventStarted == true && is_connected == true && is_notified == false) {
            gpio_set_level(SENSOR_TRIGGER_PIN, 1);
            ets_delay_us(15);
            gpio_set_level(SENSOR_TRIGGER_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(25));

            ticks_num = echoEndTime - echoStartTime;
            echoStartTime = echoEndTime;
            ticks_duration = (float)ticks_num / 1.0;
            distance = ((ticks_duration / 1000000.0) * 343.0) / 2.0;
            printf("Distance: %2f\n", distance);
            if (distance <= 0.4 && is_notified == false) {
                printf("Distance: %.3f\n", distance);
                is_notified = true;
                notify(connection_handle, 1);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void push_button_task(void *parameters) {
    int ble_button_state = 0;
    AppState ble_state = STATE_IDLE;

    gpio_config(&io_conf_ble_start);

    while (true) {
        ble_button_state = gpio_get_level(BLE_START_PIN);
        switch (ble_state) {
            case STATE_IDLE:
                if (ble_button_state == 0) {
                    vTaskDelay(pdMS_TO_TICKS(500));
                    printf("BLE service started.\n");
                    ble_app_advertise();
                    ble_state = STATE_STARTED;
                }
                break;

            case STATE_STARTED:
                if (ble_button_state == 0) {
                    vTaskDelay(pdMS_TO_TICKS(500));
                    if (is_connected) {
                        ble_gap_terminate(connection_handle, 0);
                    }
                    is_connected = false;
                    advertising_in_progress = false;
                    ble_gap_adv_stop();
                    printf("BLE service ended.\n");
                    ble_state = STATE_IDLE;
                }
                break;

            default:
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void event_start_task(void *parameters){
    int event_button_state = 0;
    gpio_config(&io_conf_event_start);

    while(true) {
        event_button_state = gpio_get_level(EVENT_START_PIN);
        vTaskDelay(pdMS_TO_TICKS(200));
        if(event_button_state == 0 && eventStarted == false) {
            eventStarted = true;
        } else if(event_button_state == 0 && eventStarted == true) {
            eventStarted = false;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}