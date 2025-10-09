#include "gpio.h"

/* internal error latch (not thread-safe; good enough for bare-metal) */
static volatile int32_t s_last_error = 0;

/* enable the gpio clock that matches the given port */
static void gpio_enable_clock(GPIO_TypeDef *port)
{
    if (port == GPIOA)
        __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB)
        __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC)
        __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD)
        __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE)
        __HAL_RCC_GPIOE_CLK_ENABLE();

    /* afio is used on f1 for exti mapping and some remaps */
    __HAL_RCC_AFIO_CLK_ENABLE();
}

int32_t gpio_last_error(void)
{
    return s_last_error;
}

gpio_status_t gpio_setup_output(const gpio_pin_t *p, uint32_t speed, bool initial_on)
{
    if (!p || !p->port)
    {
        s_last_error = -100;
        return gpio_err;
    }

    gpio_enable_clock(p->port);

    /* set initial level before switching to output to avoid glitches */
    HAL_GPIO_WritePin(p->port, p->pin, initial_on ? GPIO_PIN_SET : GPIO_PIN_RESET);

    GPIO_InitTypeDef cfg = {0};
    cfg.Pin = p->pin;
    cfg.Mode = GPIO_MODE_OUTPUT_PP; /* push-pull */
    cfg.Pull = GPIO_NOPULL;
    cfg.Speed = speed; /* e.g. GPIO_SPEED_FREQ_LOW */

    /* stm32f1 hal: hal_gpio_init returns void */
    HAL_GPIO_Init(p->port, &cfg);

    return gpio_ok;
}

void gpio_set(const gpio_pin_t *p, bool on)
{
    if (!p || !p->port)
    {
        s_last_error = -102;
        return;
    }
    HAL_GPIO_WritePin(p->port, p->pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool gpio_get(const gpio_pin_t *p)
{
    if (!p || !p->port)
    {
        s_last_error = -103;
        return false;
    }
    return (HAL_GPIO_ReadPin(p->port, p->pin) == GPIO_PIN_SET);
}

void gpio_toggle(const gpio_pin_t *p)
{
    if (!p || !p->port)
    {
        s_last_error = -104;
        return;
    }
    HAL_GPIO_TogglePin(p->port, p->pin);
}

gpio_status_t gpio_setup_input_irq(const gpio_pin_t *p, uint32_t mode, uint32_t pull, uint32_t prio)
{
    if (!p || !p->port)
    {
        s_last_error = -105;
        return gpio_err;
    }

    gpio_enable_clock(p->port);

    /* configure the pin as exti; on f1 hal will also update afio->exticr */
    GPIO_InitTypeDef cfg = {0};
    cfg.Pin = p->pin;
    cfg.Mode = mode; /* GPIO_MODE_IT_RISING / _FALLING / _RISING_FALLING */
    cfg.Pull = pull; /* GPIO_NOPULL / GPIO_PULLUP / GPIO_PULLDOWN */

    /* stm32f1 hal: hal_gpio_init returns void */
    HAL_GPIO_Init(p->port, &cfg);

    /* select nvic line based on pin number */
    IRQn_Type irqn;
    switch (p->pin)
    {
        case GPIO_PIN_0:
            irqn = EXTI0_IRQn;
            break;
        case GPIO_PIN_1:
            irqn = EXTI1_IRQn;
            break;
        case GPIO_PIN_2:
            irqn = EXTI2_IRQn;
            break;
        case GPIO_PIN_3:
            irqn = EXTI3_IRQn;
            break;
        case GPIO_PIN_4:
            irqn = EXTI4_IRQn;
            break;
        case GPIO_PIN_5:
        case GPIO_PIN_6:
        case GPIO_PIN_7:
        case GPIO_PIN_8:
        case GPIO_PIN_9:
            irqn = EXTI9_5_IRQn;
            break;
        default:
            irqn = EXTI15_10_IRQn;
            break;
    }

    HAL_NVIC_SetPriority(irqn, prio, 0);
    HAL_NVIC_EnableIRQ(irqn);

    return gpio_ok;
}

/* weak default hook: user may override in application code */
__attribute__((weak)) void gpio_on_interrupt(uint16_t pin)
{
    (void) pin;
    /* user can implement this function to handle exti events */
}

/* hal callback → route to user hook */
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    gpio_on_interrupt(pin);
}
