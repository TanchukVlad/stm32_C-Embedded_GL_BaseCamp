#include "tick.h"

inline sk_attr_alwaysinline void __WFI(void)
{
        __asm__ volatile ("WFI");
}


volatile uint32_t __sk_nticks;

void sk_attr_weak sys_tick_handler(void)
{
        __sk__nticks_inc();
}


bool sk_tick_init(uint32_t rel_val, uint8_t priority)
{
        if (rel_val & 0xFF000000)
        {
                // Bits 31:24 Reserved, must be kept cleared. More Programing manual p.248
                return false;
        }

        bool div8 = false;
        if (!(rel_val % 8))
        {
                rel_val /= 8;
                div8 = true;
        }

        // Disable SysTick.
        systick_counter_disable();
        // Selects the clock source. Processor clock (AHB).
        systick_set_clocksource(div8 ? STK_CSR_CLKSOURCE_AHB_DIV8  : STK_CSR_CLKSOURCE_AHB);
        systick_interrupt_enable();
        // The RELOAD value can be any value in the range 0x00000001-0x00FFFFFF.
        systick_set_reload(rel_val);
        // SysTick current value register.
        // A write of any value clears the field to 0, and also clears the COUNTFLAG bit in the
        // STK_CTRL register to 0.
        STK_CVR = rel_val;
        nvic_set_priority(NVIC_SYSTICK_IRQ, priority);
        nvic_enable_irq(NVIC_SYSTICK_IRQ);
        __sk_nticks = 0;
        // Enable SysTick.
        systick_counter_enable();

        return true;
}


uint32_t sk_tick_get_rate_hz(void)
{
        // STK_CSR = clocksource & STK_CSR_CLKSOURCE;
        // clocksource (1 << 0) -> div8
        // clocksource (1 << 2) -> without div 8
        // clocksource = STK_CSR & STK_CSR_CLKSOURCE = 0 -> div8
        if (!(STK_CSR & STK_CSR_CLKSOURCE))
        {
                return (rcc_ahb_frequency/systick_get_reload()) / 8;
        }
        return rcc_ahb_frequency/systick_get_reload();
}


void sk_tick_delay_ms(uint32_t ms)
{
        uint32_t cur = sk_tick_get_current();
        uint32_t delta = (sk_tick_get_rate_hz() / 1000) * ms;
        uint32_t next = cur + delta;


        // cur = 14; del = 30; next = 30 + 14 = 44 > 32 -> overflow -> 44 - 32 = 12;
        //              cur
        // *************|****************** <-overflow
        //        next |<------(c>n)----->|
        // ***********|********************
        //|<--n>c--->|
        if (sk_tick_get_current() > next) {
                while (sk_tick_get_current() > next)
                        __WFI();
        } else {
                while (sk_tick_get_current() <= next)
                        __WFI();
        }
}
