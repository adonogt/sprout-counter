#include "stm32f1xx_hal.h"
#include "app_setup.h"
#include "drv_gpio.h"

/* Entry point of the application */
int main(void)
{
    HAL_Init();      /* SysTick, NVIC base config */
    app_setup();     /* Clock + GPIO init */
    while (1) {
        drv_gpio_toggle(LED_STATUS_PIN_PORT, LED_STATUS_PIN);
        HAL_Delay(500);
    }
}
