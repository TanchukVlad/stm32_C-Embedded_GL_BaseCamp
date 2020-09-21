#include "freq_read.h"

volatile uint32_t __freq;

void timer3_init(void)
{
        rcc_periph_clock_enable(RCC_TIM3);
        // Fin = 84MHz
        // div = 84e6 / 2e3 = 42000
        uint16_t div = rcc_apb1_frequency * 2 / 10000;
        timer_set_prescaler(TIM3, div - 1);
        timer_set_counter(TIM3, 0);
        timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        timer_one_shot_mode(TIM3);
        timer_set_master_mode(TIM3, TIM_CR2_MMS_ENABLE);  //!!!!
        //Set Master/Slave mode
        TIM_SMCR(TIM3) |= TIM_SMCR_MSM;
        timer_disable_preload(TIM3);
        // Set URS bit to update only at overflow (when auto-reload value is reached)
        timer_update_on_overflow(TIM3);
        // Enable interrupt
        timer_enable_irq(TIM3, TIM_DIER_UIE);
        // Reset interrupt
        timer_clear_flag(TIM3, TIM_SR_UIF);
        nvic_set_priority(NVIC_TIM3_IRQ, 2);
        nvic_enable_irq(NVIC_TIM3_IRQ);
}


void timer2_init(void)
{
        rcc_periph_clock_enable(RCC_TIM2);
        timer_set_prescaler(TIM2, 0);
        // Max period for TIM2
        timer_set_period(TIM2, 4294967295);
        timer_set_counter(TIM2, 0);
        timer_set_master_mode(TIM2, TIM_CR2_MMS_UPDATE);
        //timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        timer_slave_set_filter(TIM2, TIM_IC_OFF);
        timer_slave_set_prescaler(TIM2, TIM_IC_PSC_OFF);
        timer_slave_set_polarity(TIM2, TIM_ET_RISING);
        //External clock mode 2 enabled. The counter is clocked by any active edge on the ETRF signal.
        TIM_SMCR(TIM2) |= (1 << 14);
        // Set Trigger selection
        // For TIM2 internal trigger connection TIM3_TRGO -> ITR2 (TS = 010)
        timer_slave_set_trigger(TIM2, TIM_SMCR_TS_ITR2);
        //Set Gated Mode - The counter clock is enabled when the trigger input (TRGI) is high.
        timer_slave_set_mode(TIM2, TIM_SMCR_SMS_GM);
}


void timer4_init(void)
{
        rcc_periph_clock_enable(RCC_TIM4);
        timer_set_prescaler(TIM4, 0);
        timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        timer_slave_set_filter(TIM2, TIM_IC_OFF);
        timer_slave_set_prescaler(TIM2, TIM_IC_PSC_OFF);
        timer_slave_set_polarity(TIM2, TIM_ET_RISING);
        // Set Trigger selection
        // For TIM2 internal trigger connection TIM3_TRGO -> ITR2 (TS = 010)
        timer_slave_set_trigger(TIM2, TIM_SMCR_TS_MASK);
}


uint32_t freq_read(uint32_t ms)
{
        // return fast
	if (!ms)
		return 0;
        timer_set_period(TIM3, ms*10 - 1);
        timer_generate_event(TIM3, TIM_EGR_UG);

        timer_enable_counter(TIM3);
        timer_enable_counter(TIM2);
        timer_enable_counter(TIM4);
        __asm__ volatile ("wfi");
        return  (__freq * 1000 / ms);
}


void tim3_isr(void)
{
        timer_clear_flag(TIM3, TIM_SR_UIF);
        __freq = timer_get_counter(TIM2) +
                 (timer_get_counter(TIM4) * TIM_ARR(TIM2)) + timer_get_counter(TIM4);
        timer_set_counter(TIM2, 0);
        timer_set_counter(TIM3, 0);
        timer_set_counter(TIM4, 0);
}
