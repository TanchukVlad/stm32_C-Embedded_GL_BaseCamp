#include "pin.h"
#include "macro.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>

const sk_pin led_red = { .port=PORTD, .pin=14, .isinverse=false};

inline sk_attr_alwaysinline void __WFI(void)
{
        __asm__ volatile ("WFI"); // Wait for interrupt.
}

volatile uint32_t __nticks = 0;

inline sk_attr_alwaysinline uint32_t __nticks_get_current(void)
{
        return __nticks;
}

inline sk_attr_alwaysinline void __nticks_inc(void)
{
        __nticks++;
}

const uint32_t TICK_RATE_HZ = 10000ul;
/**
 * System handler for SysTick exception.
 */
void sk_attr_weak sys_tick_handler(void)
{
        __nticks_inc();
}


void systick_init(void)
{
        // Disable SysTick.
        systick_counter_disable();

        // Selects the clock source. Processor clock (AHB), so f = 16MHz.
        systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);

        systick_interrupt_enable();
        // The RELOAD value can be any value in the range 0x00000001-0x00FFFFFF.
        // We need tick rate = 100us, so rel_val = 100e-6 * 16e6 = 1600.
        // f_tr = 1/100e-6 = 10000;
        uint32_t rel_val = 16000000ul/TICK_RATE_HZ;
        systick_set_reload(rel_val);

        // SysTick current value register.
        //A write of any value clears the field to 0, and also clears the COUNTFLAG bit in the
        //STK_CTRL register to 0.
        STK_CVR = rel_val;
        // Enable SysTick.

        nvic_set_priority(NVIC_SYSTICK_IRQ, 1);
        nvic_enable_irq(NVIC_SYSTICK_IRQ);

        __nticks = 0;

        systick_counter_enable();
}


static void delay_ms_systick(uint32_t ms)
{
        uint32_t cur = __nticks_get_current();
        uint32_t delta = (TICK_RATE_HZ / 1000) * ms;
        uint32_t next = cur + delta;

        while (__nticks_get_current() <= next) {
                __WFI();
        }
}


int main(void) {

        rcc_periph_clock_enable(RCC_GPIOD);

        sk_pin_mode_setup(led_red, MODE_OUTPUT);

        sk_pin_set(led_red, true);

        systick_init();
        cm_enable_interrupts();

        while (1) {
                delay_ms_systick(500);
                sk_pin_toggle(led_red);
        }
}
