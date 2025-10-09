#ifndef IR_H
#define IR_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"

/* fixed mapping: pa0→ir0, pa1→ir1, pa2→ir2 */
typedef enum {
  ir0 = 0,
  ir1 = 1,
  ir2 = 2,
  ir_count = 3
} ir_id_t;

/* initialize pa0/pa1/pa2 as input with exti (falling edge by default) */
bool ir_init(void);

/* counters api */
uint32_t ir_get_count(ir_id_t id);
void     ir_reset_count(ir_id_t id);
uint32_t ir_get_total(void);
void     ir_reset_all(void);

/* optional user hook: called on each accepted event after debounce
 * level is the sampled logical level after the interrupt edge
 */
__attribute__((weak)) void ir_on_event(ir_id_t id, bool level);

#endif /* IR_H */
