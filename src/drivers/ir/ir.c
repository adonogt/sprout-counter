
#include "ir.h"

/* software dead-time to mitigate bounce/noise (ms) */
#ifndef IR_DEBOUNCE_MS
#define IR_DEBOUNCE_MS 3u
#endif

/* counters and last-event timestamps per channel */
static volatile uint32_t s_cnt[ir_count] = {0, 0, 0};
static volatile uint32_t s_last_ms[ir_count] = {0, 0, 0};

/* map gpio pin bit to ir_id */
static inline ir_id_t pin_to_id(uint16_t pin)
{
    switch (pin)
    {
        case GPIO_PIN_0:
            return ir0;
        case GPIO_PIN_1:
            return ir1;
        case GPIO_PIN_2:
            return ir2;
        default:
            return (ir_id_t) 255;
    }
}

/* configure pa0/pa1/pa2 as input with exti falling edge (beam break = low) */
bool ir_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* exti 0..2 are already connected to port a by default on f1 */
    GPIO_InitTypeDef gi = {0};
    gi.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    gi.Mode = GPIO_MODE_IT_FALLING; /* adjust to RISING or RISING_FALLING if needed */
    gi.Pull = GPIO_PULLUP;          /* typical ir modules: open-collector low-active */
    HAL_GPIO_Init(GPIOA, &gi);

    /* irq priorities kept low to avoid starving other isrs */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_SetPriority(EXTI1_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_SetPriority(EXTI2_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    return true;
}

/* this function is called from the gpio exti path:
 * hal: HAL_GPIO_EXTI_IRQHandler(pin) → HAL_GPIO_EXTI_Callback(pin)
 * your board layer should call gpio_on_interrupt(pin) there.
 * we implement gpio_on_interrupt() here to consume pa0..pa2.
 */
void gpio_on_interrupt(uint16_t gpio_pin)
{
    ir_id_t id = pin_to_id(gpio_pin);
    if (id >= ir_count)
    {
        return;
    }

    uint32_t now = HAL_GetTick();
    uint32_t last = s_last_ms[id];
    if ((now - last) < IR_DEBOUNCE_MS)
    {
        /* ignore if inside dead-time */
        return;
    }
    s_last_ms[id] = now;

    /* sample pin level after edge */
    bool level = (HAL_GPIO_ReadPin(GPIOA, gpio_pin) == GPIO_PIN_SET);

    /* increment counter on accepted event */
    s_cnt[id]++;

    /* optional user hook */
    ir_on_event(id, level);
}

/* counters api */
uint32_t ir_get_count(ir_id_t id)
{
    if (id >= ir_count)
        return 0;
    return s_cnt[id];
}

void ir_reset_count(ir_id_t id)
{
    if (id >= ir_count)
        return;
    s_cnt[id] = 0;
}

uint32_t ir_get_total(void)
{
    return s_cnt[0] + s_cnt[1] + s_cnt[2];
}

void ir_reset_all(void)
{
    s_cnt[0] = s_cnt[1] = s_cnt[2] = 0;
}

/* default weak hook; user can override elsewhere */
void __attribute__((weak)) ir_on_event(ir_id_t id, bool level)
{
    (void) id;
    (void) level;
    /* no-op */
}
