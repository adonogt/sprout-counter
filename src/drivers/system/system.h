#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "drivers/i2c/i2c.h"         /* generic i2c api */
#include "drivers/display/display.h" /* display driver (u8g2 over i2c) */

#ifdef __cplusplus
extern "C"
{
#endif

    /* clock tree config (hse->pll 72mhz by default, hsi fallback option in .c) */
    void system_clock_config(void);

    /* hal init, clock, board gpio, i2c1 (pb6/pb7 @ 400 khz), and display */
    void system_init(void);

    /* weak error loop hook (blinks led fast) */
    void system_error_loop(void);

    /* accessor: returns initialized i2c1 bus or null if init failed */
    i2c_bus_t *system_i2c1(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_H */
