/* all comments in lowercase */
#include "drivers/i2c/i2c.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"
#include <string.h>

/* internal helpers */
static i2c_status_t _map_hal(HAL_StatusTypeDef s)
{
    switch (s) {
    case HAL_OK:      return I2C_ST_OK;
    case HAL_BUSY:    return I2C_ST_BUSY;
    case HAL_TIMEOUT: return I2C_ST_TIMEOUT;
    default:          return I2C_ST_HALERR;
    }
}

static void _enable_clocks(const i2c_config_t *cfg)
{
    /* enable gpio clocks for both ports used */
    if (cfg->scl_port == GPIOA || cfg->sda_port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    if (cfg->scl_port == GPIOB || cfg->sda_port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    if (cfg->scl_port == GPIOC || cfg->sda_port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
#ifdef GPIOD
    if (cfg->scl_port == GPIOD || cfg->sda_port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
#endif
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* i2c peripheral clock */
    if (cfg->instance == I2C1) {
        __HAL_RCC_I2C1_CLK_ENABLE();
    } else if (cfg->instance == I2C2) {
        __HAL_RCC_I2C2_CLK_ENABLE();
    }
}

static void _configure_pins(const i2c_config_t *cfg)
{
    GPIO_InitTypeDef pin = {0};
    pin.Mode  = GPIO_MODE_AF_OD;
    pin.Speed = GPIO_SPEED_FREQ_HIGH;
    pin.Pull  = GPIO_NOPULL; /* external pull-ups expected on sda/scl */

    pin.Pin = cfg->scl_pin;
    HAL_GPIO_Init(cfg->scl_port, &pin);

    pin.Pin = cfg->sda_pin;
    HAL_GPIO_Init(cfg->sda_port, &pin);

    /* i2c1 optional remap to pb8/pb9 */
    if (cfg->instance == I2C1) {
        if (cfg->i2c1_remap) {
            __HAL_AFIO_REMAP_I2C1_ENABLE();
        } else {
            __HAL_AFIO_REMAP_I2C1_DISABLE();
        }
    }
}

i2c_status_t i2c_init(i2c_bus_t *bus, const i2c_config_t *cfg)
{
    if (!bus || !cfg || (cfg->instance != I2C1 && cfg->instance != I2C2))
        return I2C_ST_PARAM;

    memset(bus, 0, sizeof(*bus));
    bus->cfg = *cfg;

    _enable_clocks(cfg);
    _configure_pins(cfg);

    /* deinit before init to ensure a clean state */
    bus->hi2c.Instance             = cfg->instance;
    (void)HAL_I2C_DeInit(&bus->hi2c);

    /* f1 init fields */
    bus->hi2c.Init.ClockSpeed      = cfg->clock_speed_hz ? cfg->clock_speed_hz : 400000U;
    bus->hi2c.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    bus->hi2c.Init.OwnAddress1     = 0x00;
    bus->hi2c.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    bus->hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    bus->hi2c.Init.OwnAddress2     = 0x00;
    bus->hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    bus->hi2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    HAL_StatusTypeDef st = HAL_I2C_Init(&bus->hi2c);
    if (st != HAL_OK) return _map_hal(st);

    bus->ready = true;
    return I2C_ST_OK;
}

i2c_status_t i2c_write(i2c_bus_t *bus, uint8_t addr7, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (!bus || !bus->ready || (addr7 > 0x7f) || (len && !data)) return I2C_ST_PARAM;
    HAL_StatusTypeDef st = HAL_I2C_Master_Transmit(&bus->hi2c, (uint16_t)(addr7 << 1), (uint8_t*)data, (uint16_t)len, timeout_ms);
    return _map_hal(st);
}

i2c_status_t i2c_read(i2c_bus_t *bus, uint8_t addr7, uint8_t *data, size_t len, uint32_t timeout_ms)
{
    if (!bus || !bus->ready || (addr7 > 0x7f) || (len && !data)) return I2C_ST_PARAM;
    HAL_StatusTypeDef st = HAL_I2C_Master_Receive(&bus->hi2c, (uint16_t)(addr7 << 1), data, (uint16_t)len, timeout_ms);
    return _map_hal(st);
}

i2c_status_t i2c_write_read(i2c_bus_t *bus, uint8_t addr7,
                            const uint8_t *wbuf, size_t wlen,
                            uint8_t *rbuf, size_t rlen,
                            uint32_t timeout_ms)
{
    if (!bus || !bus->ready || (addr7 > 0x7f)) return I2C_ST_PARAM;

    HAL_StatusTypeDef st;
    if (wlen) {
        if (!wbuf) return I2C_ST_PARAM;
        st = HAL_I2C_Master_Transmit(&bus->hi2c, (uint16_t)(addr7 << 1), (uint8_t*)wbuf, (uint16_t)wlen, timeout_ms);
        if (st != HAL_OK) return _map_hal(st);
    }
    if (rlen) {
        if (!rbuf) return I2C_ST_PARAM;
        st = HAL_I2C_Master_Receive(&bus->hi2c, (uint16_t)(addr7 << 1), rbuf, (uint16_t)rlen, timeout_ms);
        if (st != HAL_OK) return _map_hal(st);
    }
    return I2C_ST_OK;
}

i2c_status_t i2c_is_ready(i2c_bus_t *bus, uint8_t addr7, uint8_t trials, uint32_t timeout_ms)
{
    if (!bus || !bus->ready || (addr7 > 0x7f)) return I2C_ST_PARAM;
    HAL_StatusTypeDef st = HAL_I2C_IsDeviceReady(&bus->hi2c, (uint16_t)(addr7 << 1), trials ? trials : 1, timeout_ms);
    return _map_hal(st);
}

