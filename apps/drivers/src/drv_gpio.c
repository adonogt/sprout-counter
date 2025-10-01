#include "drv_gpio.h"

void drv_gpio_init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef io = {0};
    io.Pin   = LED_STATUS_PIN;
    io.Mode  = GPIO_MODE_OUTPUT_PP;
    io.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_STATUS_PIN_PORT, &io);

    /* LED off by default (active-low on PC13). */
    HAL_GPIO_WritePin(LED_STATUS_PIN_PORT, LED_STATUS_PIN, GPIO_PIN_SET);
}

void drv_gpio_toggle(GPIO_TypeDef *port, uint16_t pin)
{
    HAL_GPIO_TogglePin(port, pin);
}
