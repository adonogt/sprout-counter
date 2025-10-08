#include "drivers/system/system.h"
#include "drivers/gpio/gpio.h"

static const gpio_pin_t led_pc13 = { GPIOC, GPIO_PIN_13 };

int main(void)
{
  system_init();
  while (1) {
    gpio_toggle(&led_pc13);
    HAL_Delay(100);
  }
}
