#ifndef SENSOR_H
#define SENSOR_H

#include <stdio.h>
#include <inttypes.h>
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "rom/ets_sys.h"
#include "esp_attr.h"



#define SENSOR_TRIGGER_PIN      GPIO_NUM_0
#define BLE_START_PIN           GPIO_NUM_8
#define EVENT_START_PIN         GPIO_NUM_2
#define ECHO_PIN                GPIO_NUM_3

extern const char *PBTN;

extern uint64_t echoStartTime;
extern uint64_t echoEndTime;
extern bool echoPulseStarted;
extern bool eventStarted;

extern gptimer_handle_t gptimer;
extern gptimer_config_t timer;
extern gpio_config_t io_conf_ble_start;
extern gpio_config_t io_conf_sensor;
extern gpio_config_t io_conf_echo;
extern gpio_config_t io_conf_event_start;

typedef enum {
    STATE_IDLE,
    STATE_STARTED
} AppState;

void push_button_task(void *parameters);
void sensor_task(void *parameters);
void event_start_task(void *parameters);
void IRAM_ATTR echoPinHandler(void *arg);

#endif /*SENSOR_H*/