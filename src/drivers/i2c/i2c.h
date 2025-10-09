#ifndef I2C_GENERIC_H
#define I2C_GENERIC_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* status codes */
    typedef enum
    {
        I2C_ST_OK = 0,
        I2C_ST_BUSY,
        I2C_ST_TIMEOUT,
        I2C_ST_PARAM,
        I2C_ST_HALERR
    } i2c_status_t;

    /* configuration for an i2c bus instance */
    typedef struct
    {
        I2C_TypeDef *instance;   /* I2C1 or I2C2 */
        uint32_t clock_speed_hz; /* 100k/400k typical */
        GPIO_TypeDef *scl_port;
        uint16_t scl_pin;
        GPIO_TypeDef *sda_port;
        uint16_t sda_pin;
        bool i2c1_remap; /* true = remap i2c1 to pb8/pb9; ignored for i2c2 */
    } i2c_config_t;

    /* handle holding hal handle + copy of config */
    typedef struct
    {
        I2C_HandleTypeDef hi2c;
        i2c_config_t cfg;
        bool ready;
    } i2c_bus_t;

    /* api */
    i2c_status_t i2c_init(i2c_bus_t *bus, const i2c_config_t *cfg);
    i2c_status_t
    i2c_write(i2c_bus_t *bus, uint8_t addr7, const uint8_t *data, size_t len, uint32_t timeout_ms);
    i2c_status_t
    i2c_read(i2c_bus_t *bus, uint8_t addr7, uint8_t *data, size_t len, uint32_t timeout_ms);
    i2c_status_t i2c_write_read(i2c_bus_t *bus,
                                uint8_t addr7,
                                const uint8_t *wbuf,
                                size_t wlen,
                                uint8_t *rbuf,
                                size_t rlen,
                                uint32_t timeout_ms);
    i2c_status_t i2c_is_ready(i2c_bus_t *bus, uint8_t addr7, uint8_t trials, uint32_t timeout_ms);

    /* optional: expose hal handle for advanced users */
    static inline I2C_HandleTypeDef *i2c_hal(i2c_bus_t *bus)
    {
        return &bus->hi2c;
    }

#ifdef __cplusplus
}
#endif
#endif /* I2C_GENERIC_H */
