#include "pin.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

inline sk_attr_alwaysinline void __WFI(void)
{
        __asm__ volatile ("WFI"); // Wait for interrupt.
}

const sk_pin led_green = { .port=PORTD, .pin=12, .isinverse=false};

/**
 * User interrupt service routines (ISR) for TIM7
 * F_in = 16 MHz, t = 1 us
 * We are seting the value = 16 in TIM7 prescaler register (TIMx_PSC)
 * The prescaler can divide the counter clock frequency between 1 and 65536.
 */
void delay_timer_init(void)
{
        //Timer config:
        //The counter clock frequency CK_CNT is equal to f_CK_PSC/(PSC[15:0]+1).
        timer_set_prescaler(TIM7, 16 - 1);  //Set prescaler value = 16

        // Enable interrupt
        timer_enable_irq(TIM7, TIM_DIER_UIE);
        // Reset interrupt
        timer_clear_flag(TIM7, TIM_SR_UIF);

        //In TIM7 control register 1 (TIM7_CR1) resets Bit 7 ARPE: Auto-reload preload enable
        // 0: TIM7_ARR register is not buffered.
        timer_disable_preload(TIM7);
        // In TIM7 control register 1 (TIM7_CR1) sets Bit 3 OPM: One-pulse mode
        // 1: Counter stops counting at the next update event (clearing the CEN bit).
        timer_one_shot_mode(TIM7);

        // Set URS bit to update only at overflow (when auto-reload value is reached)
        timer_update_on_overflow(TIM7);

        nvic_set_priority(NVIC_TIM7_IRQ, 2);
        nvic_enable_irq(NVIC_TIM7_IRQ);
}


void delay_us(uint32_t val)
{
        if (!val)
                return;

        timer_set_period(TIM7, val);

        // generate update event to transfer ARPE contents to shadow register
	timer_generate_event(TIM7, TIM_EGR_UG);

        __dmb(); // data memory barrier (asm instuction DMB)
        // DMB acts as a data memory barrier. It ensures that all explicit memory accesses that
        // appear, in program order, before the DMB instruction are completed before any explicit
        // memory accesses that appear, in program order, after the DMB instruction. DMB does not
        // affect the ordering or execution of instructions that do not access memory.

        timer_enable_counter(TIM7);
        __WFI();	// Enter sleep mode. WFI -- wait for interrupt
}

static void delay_ms(uint32_t val)
{
	while (val--)
		delay_us(1000);
}

void tim7_isr(void)
{
        timer_clear_flag(TIM7, TIM_SR_UIF);
}

int main(void)
{

        rcc_periph_clock_enable(RCC_GPIOD);
        rcc_periph_clock_enable(RCC_TIM7);

        sk_pin_mode_setup(led_green, MODE_OUTPUT);

        delay_timer_init();
        cm_enable_interrupts();

        sk_pin_set(led_green, true);  //led on

        while (1) {
                delay_ms(1000);
                sk_pin_toggle(led_green);
        }
}
