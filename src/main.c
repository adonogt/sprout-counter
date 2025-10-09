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
    system_init(); 

    if (!ir_init())
    { 
        system_error_loop();
    }

    while (1)
    {
    }
}
