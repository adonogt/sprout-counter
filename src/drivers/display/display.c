#include "drivers/display/display.h"
#include "drivers/system/system.h"
#include "drivers/i2c/i2c.h"
#include "stm32f1xx_hal.h"
#include <string.h>

/* address note: many 128x32 ssd1306 boards use 0x3c; some use 0x3d */
#ifndef SSD1306_ADDR_7BIT
#define SSD1306_ADDR_7BIT  0x3C
#endif

/* local state */
static u8g2_t s_u8g2;
static bool   s_ready = false;

/* ---- u8g2 callbacks (embedded here) -------------------------------------- */

static i2c_bus_t *bus(void) { return system_i2c1(); }

/* i2c byte callback (u8g2 → our generic i2c, polling) */
static uint8_t u8x8_byte_stm32_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t txbuf[32];
    static uint8_t idx;

    switch (msg) {
    case U8X8_MSG_BYTE_INIT:
        /* bus is already initialized by system_init() */
        return 1;

    case U8X8_MSG_BYTE_START_TRANSFER:
        idx = 0;
        return 1;

    case U8X8_MSG_BYTE_SEND: {
        uint8_t *data = (uint8_t *)arg_ptr;
        while (arg_int--) {
            if (idx >= sizeof(txbuf)) {
                (void)i2c_write(bus(), (uint8_t)(u8x8_GetI2CAddress(u8x8) >> 1), txbuf, idx, 10);
                idx = 0;
            }
            txbuf[idx++] = *data++;
        }
        return 1;
    }

    case U8X8_MSG_BYTE_END_TRANSFER:
        if (idx) {
            (void)i2c_write(bus(), (uint8_t)(u8x8_GetI2CAddress(u8x8) >> 1), txbuf, idx, 20);
            idx = 0;
        }
        return 1;

    default:
        return 0;
    }
}

/* gpio/delay callback; dc/reset not used for i2c panels */
static uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    (void)u8x8; (void)arg_ptr;

    switch (msg) {
    case U8X8_MSG_DELAY_MILLI:
        HAL_Delay((uint32_t)arg_int);
        return 1;

    case U8X8_MSG_DELAY_10MICRO: {
        /* crude busy-wait; good enough for tiny delays */
        volatile uint32_t n = (uint32_t)arg_int * 30U;
        while (n--) { __NOP(); }
        return 1;
    }

    case U8X8_MSG_GPIO_DC:
    case U8X8_MSG_GPIO_RESET:
    case U8X8_MSG_GPIO_CS:
    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
        return 1;

    default:
        return 1;
    }
}

/* ---- public api ----------------------------------------------------------- */

bool display_init(void)
{
    if (bus() == NULL) {
        s_ready = false;
        return false;
    }

    /* configure u8g2 for ssd1306 128x32 i2c, full buffer */
    u8g2_Setup_ssd1306_i2c_128x32_univision_f(&s_u8g2, U8G2_R0,
        u8x8_byte_stm32_i2c, u8x8_gpio_and_delay_stm32);

    /* u8g2 expects 8-bit address; shift our 7-bit */
    u8x8_SetI2CAddress(&s_u8g2.u8x8, (uint8_t)(SSD1306_ADDR_7BIT << 1));

    u8g2_InitDisplay(&s_u8g2);
    u8g2_SetPowerSave(&s_u8g2, 0);

    u8g2_SetFont(&s_u8g2, u8g2_font_6x10_tf);
    u8g2_ClearBuffer(&s_u8g2);
    u8g2_DrawStr(&s_u8g2, 0, 10, "SPROUT COUNTER");
    u8g2_DrawStr(&s_u8g2, 0, 28, "FIRMWARE VERSION V1.0");
    u8g2_SendBuffer(&s_u8g2);

    s_ready = true;
    return true;
}

void display_write_text(uint8_t x, uint8_t y, const char *msg)
{
    if (!s_ready || !msg) return;
    u8g2_ClearBuffer(&s_u8g2);
    u8g2_SetFont(&s_u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&s_u8g2, x, y, msg);
    u8g2_SendBuffer(&s_u8g2);
}

void display_write_version(void)
{
    u8g2_ClearBuffer(&s_u8g2);
    u8g2_DrawStr(&s_u8g2, 0, 10, "SPROUT COUNTER");
    u8g2_DrawStr(&s_u8g2, 0, 28, "FIRMWARE VERSION V1.0");
    u8g2_SendBuffer(&s_u8g2);

}

u8g2_t* display_u8g2(void)
{
    return s_ready ? &s_u8g2 : NULL;
}

