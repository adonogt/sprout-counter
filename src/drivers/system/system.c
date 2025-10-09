#include "system.h"
#include "gpio.h"
#include "drivers/i2c/i2c.h"
#include "drivers/display/display.h"

/* bluepill user led on pc13 (active low) */
static const gpio_pin_t s_led_pc13 = {GPIOC, GPIO_PIN_13};

/* i2c1 state (pb6/pb7) */
static i2c_bus_t g_i2c1;
static bool g_i2c1_ready = false;

/* choose your clock source:
 * - option a (default): hse 8 mhz with pll to 72 mhz (common on bluepill)
 * - option b: pure hsi 8 mhz (uncomment if your hse is not populated)
 */
#define USE_CLOCK_HSE_PLL_72MHZ 1

void system_clock_config(void)
{
#if USE_CLOCK_HSE_PLL_72MHZ
    /* hse=8mhz → pll x9 → sysclk=72mhz, ahb=72, apb1=36, apb2=72 */
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.HSIState = RCC_HSI_ON; /* keep hsi on for startup safety */
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        system_error_loop();
    }

    clk.ClockType =
            RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1; /* hclk = 72 mhz */
    clk.APB1CLKDivider = RCC_HCLK_DIV2;  /* pclk1 = 36 mhz (<=36 max) */
    clk.APB2CLKDivider = RCC_HCLK_DIV1;  /* pclk2 = 72 mhz */
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK)
    {
        system_error_loop();
    }

    /* update systick to 1ms based on new hclk */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

#else
    /* fallback: hsi 8 mhz as system clock (no pll) */
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_OFF;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    {
        system_error_loop();
    }

    clk.ClockType =
            RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0) != HAL_OK)
    {
        system_error_loop();
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
#endif

    /* systick priority: keep lowest to not disturb isrs */
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

void system_init(void)
{
    /* hal and core init (systick @ 1khz) */
    HAL_Init();

    /* configure clock tree (72 mhz by default) */
    system_clock_config();

    /* configure board gpios: user led on pc13 (active low) */
    if (gpio_setup_output(&s_led_pc13, GPIO_SPEED_FREQ_LOW, /*initial_on=*/false) != gpio_ok)
    {
        system_error_loop();
    }

    /* init i2c1 on pb6/pb7 @ 400 khz; no remap */
    {
        i2c_config_t cfg = {.instance = I2C1,
                            .clock_speed_hz = 400000,
                            .scl_port = GPIOB,
                            .scl_pin = GPIO_PIN_6,
                            .sda_port = GPIOB,
                            .sda_pin = GPIO_PIN_7,
                            .i2c1_remap = false};
        if (i2c_init(&g_i2c1, &cfg) == I2C_ST_OK)
        {
            g_i2c1_ready = true;
        }
        else
        {
            g_i2c1_ready = false;
            /* optional: add a visible fault indication here */
        }
    }

    /* init display via u8g2 and print a banner for validation */
    if (g_i2c1_ready)
    {
        if (!display_init())
        {
            /* display failed to init; keep running */
        }
        else
        {
            display_write_version();
        }
    }
}

/* accessor: returns initialized i2c1 handle or null if init failed */
i2c_bus_t *system_i2c1(void)
{
    return g_i2c1_ready ? &g_i2c1 : NULL;
}

/* simple blocking error loop:
 * - toggles the led fast so you can see a fault happened
 * - stays here forever
 */
__attribute__((weak)) void system_error_loop(void)
{
    /* try to init led in case we faulted before gpio setup */
    (void) gpio_setup_output(&s_led_pc13, GPIO_SPEED_FREQ_LOW, false);

    while (1)
    {
        gpio_toggle(&s_led_pc13);
        HAL_Delay(100); /* ~10hz blink indicates error */
    }
}
