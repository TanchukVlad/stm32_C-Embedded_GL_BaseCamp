#include "macro.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
/**
 * Wait for interrupt.
 */
inline sk_attr_alwaysinline void __WFI(void)
{
        __asm__ volatile ("WFI"); // Wait for interrupt.
}

/**
 * Delay timer initialization.
 *
 * Context: Initializes TIM7 which used for milliseconds and microseconds delay.
 */
void delay_timer_init(void);

/**
 * Microseconds delay.
 * @val: Time in microseconds.
 *
 * Context: Makes microseconds delay uaing TIM7. During delay wait for interrupt.
 */
void delay_us(uint32_t val);

/**
 * Microseconds delay.
 * @val: Time in milliseconds.
 *
 * Context: Makes milliseconds delay uaing @delay_us(). During delay wait for interrupt.
 */
void delay_ms(uint32_t val);
