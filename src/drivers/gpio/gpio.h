#pragma once
/* simple gpio driver built on stm32f1 hal
 * comments are lowercase as requested
 */

#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* opaque status for basic error handling */
typedef enum {
  gpio_ok  = 0,
  gpio_err = -1
} gpio_status_t;

/* simple pin descriptor (port + pin mask) */
typedef struct {
  GPIO_TypeDef* port;
  uint16_t      pin;   /* e.g. GPIO_PIN_13 */
} gpio_pin_t;

/* initialize a pin as push-pull output
 * - speed: use hal macros, e.g. GPIO_SPEED_FREQ_LOW/MEDIUM/HIGH
 * - initial_on: true -> set pin high right before init
 */
gpio_status_t gpio_setup_output(const gpio_pin_t* p,
                                uint32_t speed,
                                bool initial_on);

/* write, read and toggle helpers */
void gpio_set(const gpio_pin_t* p, bool on);
bool gpio_get(const gpio_pin_t* p);
void gpio_toggle(const gpio_pin_t* p);

/* initialize a pin as input with external interrupt
 * - mode: GPIO_MODE_IT_RISING / GPIO_MODE_IT_FALLING / GPIO_MODE_IT_RISING_FALLING
 * - pull: GPIO_NOPULL / GPIO_PULLUP / GPIO_PULLDOWN
 * - prio: nvic priority (0 is highest on cortex-m3, larger = lower priority)
 */
gpio_status_t gpio_setup_input_irq(const gpio_pin_t* p,
                                   uint32_t mode,
                                   uint32_t pull,
                                   uint32_t prio);

/* optional: last error code helper (simple latch) */
int32_t gpio_last_error(void);

/* user hook called from hal exti callback (weak default).
 * implement this in your application if you want to be notified on any gpio exti.
 */
void gpio_on_interrupt(uint16_t pin);

#ifdef __cplusplus
}
#endif
