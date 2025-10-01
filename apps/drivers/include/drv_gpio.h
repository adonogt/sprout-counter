#ifndef DRV_GPIO_H
#define DRV_GPIO_H
#include "stm32f1xx_hal.h"
#include "board_pins.h"

/* GPIO driver: minimal interface for LED toggle */
void drv_gpio_init(void);
void drv_gpio_toggle(GPIO_TypeDef *port, uint16_t pin);

#endif /* DRV_GPIO_H */

