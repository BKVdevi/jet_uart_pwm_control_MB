#include <stdio.h>
#include <stdint.h>
#include "mbcontroller.h"
#include "esp_log.h"

#define MB_PORT_NUM UART_NUM_0
#define MB_SLAVE_ADDR 0x01
#define MB_DEV_SPEED 512000

/*
Определяем 16 каналов для данных получаемых по modbus

Тип HOLDING
*/
#pragma pack(push, 1)
typedef struct
{
    int16_t holding_chan0; // Будем использовать для выхода D15 (значения допустимые -100 до 100) шим 1000 до 2000мкс
    int16_t holding_chan1;
    int16_t holding_chan2;
    int16_t holding_chan3;
    int16_t holding_chan4;
    int16_t holding_chan5;
    int16_t holding_chan6;
    int16_t holding_chan7;
    int16_t holding_chan8;
    int16_t holding_chan9;
    int16_t holding_chan10;
    int16_t holding_chan11;
    int16_t holding_chan12;
    int16_t holding_chan13;
    int16_t holding_chan14;
    int16_t holding_chan15;
} chan_params_t;
#pragma pack(pop)


chan_params_t chan_params = {0};

//Вычисление смещений
#define HOLD_OFFSET(field) ((uint16_t)(offsetof(chan_params_t, field) >> 1))

#define MB_REG_CHAN_PARAMS_START         (0x0000)

#define MB_READ_MASK                        (MB_EVENT_INPUT_REG_RD \
                                                | MB_EVENT_HOLDING_REG_RD \
                                                | MB_EVENT_DISCRETE_RD \
                                                | MB_EVENT_COILS_RD)
#define MB_WRITE_MASK                       (MB_EVENT_HOLDING_REG_WR \
                                                | MB_EVENT_COILS_WR)
#define MB_READ_WRITE_MASK                  (MB_READ_MASK | MB_WRITE_MASK)

static void setup_reg_data(void)
{
    chan_params.holding_chan0 = -100;
    chan_params.holding_chan1 = -100;
    chan_params.holding_chan2 = -100;
    chan_params.holding_chan3 = -100;
    chan_params.holding_chan4 = -100;
    chan_params.holding_chan5 = -100;
    chan_params.holding_chan6 = -100;
    chan_params.holding_chan7 = -100;
    chan_params.holding_chan8 = -100;
    chan_params.holding_chan9 = -100;
    chan_params.holding_chan10 = -100;
    chan_params.holding_chan11 = -100;
    chan_params.holding_chan12 = -100;
    chan_params.holding_chan13 = -100;
    chan_params.holding_chan14 = -100;
    chan_params.holding_chan15 = -100;
}

mb_param_info_t reg_info; // keeps the Modbus registers access information
mb_register_area_descriptor_t reg_area = {0}; // Modbus register area descriptor structure

static void *mbc_slave_handle = NULL;

static portMUX_TYPE param_lock = portMUX_INITIALIZER_UNLOCKED;


static void  modbus_init(){

    mb_communication_info_t comm_config = {
        .ser_opts.port = MB_PORT_NUM,
        .ser_opts.mode = MB_RTU,
        // .ser_opts.mode = MB_ASCII,
        .ser_opts.baudrate = MB_DEV_SPEED,
        .ser_opts.parity = MB_PARITY_NONE,
        .ser_opts.uid = MB_SLAVE_ADDR,
        .ser_opts.data_bits = UART_DATA_8_BITS,
        .ser_opts.stop_bits = UART_STOP_BITS_1
    };

    ESP_ERROR_CHECK(mbc_slave_create_serial(&comm_config, &mbc_slave_handle));
    uart_driver_install(comm_config.ser_opts.port, 4096*2, 4096*2, 0, NULL, 0);
    // uart_set_pin(MB_PORT_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Инициализация пространства регисторв MODBUS
    reg_area.type = MB_PARAM_HOLDING; // Set type of register area
    reg_area.start_offset = MB_REG_CHAN_PARAMS_START; // Offset of register area in Modbus protocol
    reg_area.address = (void*)&chan_params.holding_chan0;
    reg_area.size = sizeof(chan_params);
    reg_area.access = MB_ACCESS_RW;

    ESP_ERROR_CHECK(mbc_slave_set_descriptor(mbc_slave_handle, reg_area));

    //Установка начальных параметров регисторв
    setup_reg_data();

    mbc_slave_start(mbc_slave_handle);

    // ESP_LOGI("BUS","MB OK");
    while (true)
    {
        // ESP_LOGI("BUS", "DATA");

        // (void)mbc_slave_check_event(mbc_slave_handle, MB_READ_WRITE_MASK);
        mbc_slave_get_param_info(mbc_slave_handle, &reg_info, (10));
        // const char* rw_str = (reg_info.type & MB_READ_MASK) ? "READ" : "WRITE";

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);

}