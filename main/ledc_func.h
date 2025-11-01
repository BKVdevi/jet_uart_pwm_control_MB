#include <stdio.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"


// ===================== LEDC CONFIGURATION =====================

#define LEDC_TIMER         LEDC_TIMER_0
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL       LEDC_CHANNEL_0
#define LEDC_DUTY_RES      LEDC_TIMER_16_BIT
#define LEDC_FREQUENCY     50              // 50 Hz (20ms период)
#define LEDC_GPIO_PIN      GPIO_NUM_5

// PWM диапазон: 1000-2000 мс (в пределах 20ms периода)
#define MIN_PULSE_MS       1000   // Минимальный импульс в микросекундах
#define MAX_PULSE_MS       2000   // Максимальный импульс в микросекундах
#define PERIOD_MS          20000  // Период в микросекундах (50 Hz = 20ms)

// ===================== ФУНКЦИЯ ИНИЦИАЛИЗАЦИИ LEDC =====================

void ledc_pwm_init(void)
{
    
    // Конфигурация таймера
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    // Конфигурация канала
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_GPIO_PIN,
        .duty           = 0,        // Начальное значение: 0
        .hpoint         = 0
    };
    
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    

}

// ===================== ФУНКЦИЯ КОНВЕРТАЦИИ И ВЫВОДА =====================

/**
 * Преобразует значение процента (-100 до +100) в длительность импульса
 * и устанавливает PWM сигнал на LEDC канал
 * 
 * Диапазон:
 *   -100% → 1000 мкс (минимум)
 *   0%    → 1500 мкс (центр)
 *   +100% → 2000 мкс (максимум)
 * 
 * Параметр: value (-100 до +100)
 */
static void set_pwm_percent(int16_t value)
{
    // Валидация диапазона
    if (value < -100) value = -100;
    if (value > 100) value = 100;
    
    // Расчет длительности импульса в микросекундах
    // Формула: pulse_us = 1500 + (value / 100) * 500
    // 1500 - центральное значение
    // 500 - диапазон (±500 мкс от центра)
    uint16_t pulse_us = 1500 + (value * 500) / 100;
    
    // Убедимся, что значение в диапазоне 1000-2000 мкс
    if (pulse_us < MIN_PULSE_MS) pulse_us = MIN_PULSE_MS;
    if (pulse_us > MAX_PULSE_MS) pulse_us = MAX_PULSE_MS;
    
    // Расчет duty cycle для 16-битного таймера
    // Максимальное значение: (1 << 16) - 1 = 65535
    // Duty в процентах: (pulse_us / PERIOD_MS) * 100
    // Duty в битах: (pulse_us / PERIOD_MS) * 65535
    
    uint32_t max_duty = (1 << LEDC_DUTY_RES) - 1;  // 65535 для 16-bit
    uint32_t duty = (pulse_us * max_duty) / PERIOD_MS;
    // Установка duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    
    // Применение изменений
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

// ===================== АЛЬТЕРНАТИВНАЯ ФУНКЦИЯ (по микросекундам) =====================

/**
 * Альтернативный способ: напрямую устанавливать импульс в микросекундах
 * 
 * Параметр: pulse_us (1000 до 2000 мкс)
 */
void set_pwm_microseconds(uint16_t pulse_us)
{
    // Валидация диапазона
    if (pulse_us < MIN_PULSE_MS) pulse_us = MIN_PULSE_MS;
    if (pulse_us > MAX_PULSE_MS) pulse_us = MAX_PULSE_MS;
    
    // Расчет duty cycle
    uint32_t max_duty = (1 << LEDC_DUTY_RES) - 1;  // 65535
    uint32_t duty = (pulse_us * max_duty) / PERIOD_MS;
    
    // Расчет процентов для логирования
    int16_t percent = (pulse_us - 1500) * 100 / 500;
    
    // Установка duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}