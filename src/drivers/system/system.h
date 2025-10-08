#pragma once
/* simple system layer for stm32f103 (hal)
 * comments are lowercase as requested
 */

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* initialize hal, system clock, and board gpios
 * - hal_init() sets systick @ 1 khz and resets peripherals
 * - system_clock_config() selects clock tree (72 mhz hse+pll by default)
 * - board gpio: config user led on pc13 as output (active-low on bluepill)
 */
void system_init(void);

/* expose the clock configuration if application needs to reconfigure later */
void system_clock_config(void);

/* optional: error handler (infinite loop + led blink pattern)
 * you can override this symbol in your app if desired
 */
void system_error_loop(void);

#ifdef __cplusplus
}
#endif

