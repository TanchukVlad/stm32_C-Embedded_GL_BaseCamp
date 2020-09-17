#include "freq_read.h"

volatile uint32_t __freq = 0;

void timer3_init(void)
{
        rcc_periph_clock_enable(RCC_TIM3);

        // Fin = 84MHz
        // Time freq Ft = 10KHz
        // div = 84e6 / 10e3 = 8400
        uint16_t div = rcc_apb1_frequency * 2 / 10000;
        timer_set_prescaler(TIM3, div - 1);

        // Enable interrupt
        timer_enable_irq(TIM3, TIM_DIER_UIE);
        // Reset interrupt
        timer_clear_flag(TIM3, TIM_SR_UIF);
        timer_disable_preload(TIM3);
        timer_one_shot_mode(TIM3);

        timer_set_master_mode(TIM3, TIM_CR2_MMS_ENABLE);
        //Set Master/Slave mode
        TIM_SMCR(TIM3) |= TIM_SMCR_MSM;

        // Set URS bit to update only at overflow (when auto-reload value is reached)
        timer_update_on_overflow(TIM3);

        nvic_set_priority(NVIC_TIM3_IRQ, 1);
        nvic_enable_irq(NVIC_TIM3_IRQ);
}


void timer2_init(void)
{
        rcc_periph_clock_enable(RCC_TIM2);
        timer_set_prescaler(TIM2, 0);

        //Set Gated Mode - The counter clock is enabled when the trigger input (TRGI) is high.
        timer_slave_set_mode(TIM2, TIM_SMCR_SMS_GM);
        // Set Trigger selection
        // For TIM2 internal trigger connection TIM3_TRGO -> ITR2 (TS = 010)
        timer_slave_set_trigger(TIM2, TIM_SMCR_TS_ITR2);

        timer_slave_set_polarity(TIM2, TIM_ET_RISING);
        timer_slave_set_prescaler(TIM2, TIM_IC_PSC_OFF);
        //External clock mode 2 enabled. The counter is clocked by any active edge on the ETRF signal.
        TIM_SMCR(TIM2) |= (1 << 14);

        timer_update_on_overflow(TIM2);
}


void freq_read_start(uint16_t sec)
{

        // Period per = 1s
        uint16_t per = (sec * rcc_apb1_frequency * 2) / (TIM_PSC(TIM3) * 1000);
        timer_set_period(TIM3, per - 1);
        // Max period for TIM2
        timer_set_period(TIM2, 4294967295);

        timer_generate_event(TIM3, TIM_EGR_UG);
        timer_enable_counter(TIM3);
        timer_enable_counter(TIM2);
        __asm__ volatile ("WFI");
}

uint32_t freq_read(void)
{
        return __freq;
}


void tim3_isr(void)
{
        __freq = timer_get_counter(TIM2);
        timer_clear_flag(TIM3, TIM_SR_UIF);
}
