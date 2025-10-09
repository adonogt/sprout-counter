/* main entry
 * comments are lowercase
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "system.h"
#include "drivers/ir/ir.h"
#include "drivers/display/display.h"
#include "u8g2.h"


int main(void)
{
  system_init();                 /* clocks, systick, gpio base, i2c1, display */

  if (!ir_init()) {              /* pa0/pa1/pa2 exti falling, pull-up */
    system_error_loop();
  }

  while (1) {
  }
}
