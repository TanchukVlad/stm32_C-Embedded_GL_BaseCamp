#include "macro.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * SysTick counter. Variable, which holds current tick value.
 */
extern volatile uint32_t __sk_nticks;


inline sk_attr_alwaysinline void __WFI(void)
{
        __asm__ volatile ("WFI");
}


inline sk_attr_alwaysinline void __WFE(void)
{
        __asm__ volatile ("WFE");
}


inline sk_attr_alwaysinline void __NOP(void)
{
        __asm__ volatile ("nop");
}
/**
 * Return SysTick counter value.
 */
inline sk_attr_alwaysinline uint32_t sk_tick_get_current(void)
{
        return __sk_nticks;
}


/**
 * Increment SysTick counter value.
 */
inline sk_attr_alwaysinline void __sk__nticks_inc(void)
{
        __sk_nticks++;
}


/**
 * This function Initialize SysTick.
 * @rel_val: The RELOAD value. Tick period in clock cycles.
             Must be <= 0x7fffff8 in any case.
 * @priority: SysTick interrupt priority (as set by :c:func:`nvic_set_priority`)
 * @return: true on success, false on init failure
 */
bool sk_tick_init(uint32_t rel_val, uint8_t priority);


/**
 * Returns current tick period in Hz.
 */
uint32_t sk_tick_get_rate_hz(void);

/**
 * Use tick counter to provide millisecond delays.
 */
void sk_tick_delay_ms(uint32_t ms);
