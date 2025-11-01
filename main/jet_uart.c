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

static void modbus_chan1_daemon(void){
    // Флаг для переключения светодиода
    static bool led_state = false;
    static int toggle_counter = 0;
    
    // Конфигурируем GPIO для светодиода (например, GPIO4)
    static bool gpio_configured = false;
    if (!gpio_configured) {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << GPIO_NUM_13),  // GPIO4 для светодиода
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&io_conf);
        gpio_configured = true;
        gpio_set_level(GPIO_NUM_4, 0);  // Выключить светодиод изначально
    }
    
    while (true)
    {
        int16_t data_chan1 = 0;

        //Тут доступ к регистрам modbus
        portENTER_CRITICAL(&param_lock);
        data_chan1 = chan_params.holding_chan0;
        portEXIT_CRITICAL(&param_lock);
        //Тут доступ к регистрам modbus закончен

        set_pwm_percent(data_chan1); //Установка канала

        // Переключение светодиода каждые 500 мс
        toggle_counter++;
        if (toggle_counter >= 50)  // 50 * 10 мс = 500 мс
        {
            led_state = !led_state;
            gpio_set_level(GPIO_NUM_4, led_state ? 1 : 0);
            toggle_counter = 0;
            
            ESP_LOGI("LED", "LED state: %d", led_state);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


void app_main(void)
{
    ledc_pwm_init();
    // xTaskCreate(modbus_init, "MB task", 4096, NULL, 2, NULL);
    modbus_init();

    xTaskCreate(modbus_chan1_daemon, "chan1daemon", 2048, NULL, 2, NULL);

    while (true)
    {
        // ESP_LOGI("STH", "WORK");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    
}