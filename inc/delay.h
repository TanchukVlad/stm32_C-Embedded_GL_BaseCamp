#include "macro.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

inline sk_attr_alwaysinline void __WFI(void)
{
        __asm__ volatile ("WFI"); // Wait for interrupt.
}


void delay_timer_init(void);


void delay_us(uint32_t val);


void delay_ms(uint32_t val);
