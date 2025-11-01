#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "ledc_func.h"
#include "modbus_init.h"

// ===================== CONFIGURATION =====================

#define MB_PORT_NUM      UART_NUM_0
#define MB_DEV_SPEED     115200

#define MIN_PWM_VALUE     -100
#define MAX_PWM_VALUE     100
#define CENTER_VALUE      0


void app_main(void)
{
    ledc_pwm_init();
    // xTaskCreate(modbus_init, "MB task", 4096, NULL, 2, NULL);
    modbus_init();

    while (true)
    {
        // ESP_LOGI("STH", "WORK");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
}