/**
 * Milliseconds and microseconds delay using TIM7.
 */
#include "delay.h"


void delay_timer_init(void)
{
        rcc_periph_clock_enable(RCC_TIM7);
        //Timer config:
        uint16_t div = rcc_apb1_frequency * 2 / 1000000;
        timer_set_prescaler(TIM7, div - 1);

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

        nvic_set_priority(NVIC_TIM7_IRQ, 1);
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

void delay_ms(uint32_t val)
{
	while (val--)
		delay_us(1000);
}

void tim7_isr(void)
{
        timer_clear_flag(TIM7, TIM_SR_UIF);
}
