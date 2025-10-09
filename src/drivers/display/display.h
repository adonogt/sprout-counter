#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "u8g2.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* initialize u8g2 over i2c (i2c1 on pb6/pb7) for a 128x32 ssd1306 */
    bool display_init(void);

    /* draw a text line at (x,y) using a small readable font */
    void display_write_text(uint8_t x, uint8_t y, const char *msg);
    void display_write_version(void);
    /* expose u8g2 handle for advanced drawings (returns null if not ready) */
    u8g2_t *display_u8g2(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H */
