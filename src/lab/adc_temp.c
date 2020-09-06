#include "lcd_hd44780.h"
#include "tick.h"
#include "printf.h"
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

sk_pin lcd_rs = { .port = PORTE, .pin = 7, .isinverse = false };
sk_pin lcd_rw = { .port = PORTE, .pin = 10, .isinverse = false };
sk_pin lcd_en = { .port = PORTE, .pin = 11, .isinverse = false };
sk_pin lcd_bkl = { .port = PORTE, .pin = 9, .isinverse = false };

sk_pin_group lcd_group = {
        .port = PORTE,
        .pins = (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12),
        .inversions = false
};

struct sk_lcd lcd = {
        .pin_group_data = &lcd_group,
        .pin_rs = &lcd_rs,
        .pin_rw = &lcd_rw,
        .pin_en = &lcd_en,
        .pin_bkl = &lcd_bkl,
        //.set_backlight_func = &test_bkl_func,
        .delay_func_us = NULL,
        .delay_func_ms = &sk_tick_delay_ms,
        .is4bitinterface = 1
};

const sk_pin temp_pin = { .port=PORTB, .pin=1, .isinverse=false};


static void adc_temp_init(void)
{
        rcc_periph_clock_enable(RCC_GPIOB);
        //minimize noise
        gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, temp_pin.pin);
        //gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 1 << 1);
        sk_pin_mode_setup(temp_pin, MODE_ANALOG);
        // APB2 prescaler /2
        rcc_set_ppre2(RCC_CFGR_PPRE_DIV_2);
        // ADC1 clock
        rcc_periph_clock_enable(RCC_ADC1);
        // ADC1 prescaler /8
        adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);

        // Set ADC resolution to 12 bit
        adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
        // Set left-to-right data alignment in adc result register
        adc_set_right_aligned(ADC1);

        // Set ADC regular sequence register;
        uint8_t channels[16];
        for (uint8_t i = 0; i < sk_arr_len(channels); i++) {
                // Set sampling time:
                // Set ADC sample time register: 480 cycles.
                // Fs = fadc / 480 = 1 MHz / 480 = 2083.33 Hz.
                adc_set_sample_time(ADC1, i, ADC_SMPR_SMP_480CYC);
                channels[i] = 9;
        }
        adc_set_regular_sequence(ADC1, sk_arr_len(channels), channels);
        // The EOC is set after each conversion in a sequence rather than at the end of the sequence.
        adc_eoc_after_each(ADC1);

        adc_enable_scan_mode(ADC1);
        //In this mode the ADC performs a conversion of one channel or a channel group and stops.
        adc_set_single_conversion_mode(ADC1);

        adc_enable_eoc_interrupt(ADC1);
        //The overrun interrupt is generated when data is not read from a result register before the next conversion is written.
        //If DMA is enabled, all transfers are terminated and any conversion sequence is aborted.
        adc_enable_overrun_interrupt(ADC1);

        nvic_set_priority(NVIC_ADC_IRQ, 10);
        nvic_enable_irq(NVIC_ADC_IRQ);

        adc_power_on(ADC1);
        sk_tick_delay_ms(10);
}

static volatile uint16_t __adc_avgval = 0;

void adc_isr(void)
{
        static volatile uint32_t sum = 0;
        static volatile uint32_t cnt = 0;


        if (adc_get_overrun_flag(ADC1)) {
                sum = cnt = 0;
                adc_clear_flag(ADC1, ADC_SR_OVR);
                return;
        }

        sum += adc_read_regular(ADC1) & 0x00000FFF;
        cnt++;
        if (cnt >= 16) {
                __adc_avgval = sum / cnt;
                sum = cnt = 0;
                adc_clear_flag(ADC1, ADC_SR_STRT);
        }
        adc_clear_flag(ADC1, ADC_SR_EOC);

}


static uint16_t adc_acquire(void)
{
        //This starts conversion on a set of defined regular channels if the ADC trigger is set to be a software trigger.
        //It is cleared by hardware once conversion starts.
        adc_start_conversion_regular(ADC1);

        while (adc_get_flag(ADC1, ADC_SR_STRT)) {
                __WFI();
        }
        return __adc_avgval;  //converted value after averaging in ISR
}



static inline float adc_temp_convert(uint16_t adcval)
{
        return 97.31 - 0.04039 * adcval;
}


int main (void)
{

        rcc_periph_clock_enable(RCC_GPIOE);
        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rs, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rw, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_en, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_bkl, MODE_OUTPUT);

        uint32_t period = 16000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        sk_pin_group_set(lcd_group, 0x00);

        lcd_poweron_delay(&lcd);

        adc_temp_init();

        lcd_init_4bit(&lcd);

        sk_lcd_set_backlight(&lcd, true);

        lcd_print_text(&lcd, "ADC Temp:");
        sk_lcd_set_addr(&lcd, 0x40);

        while(1)
        {
                uint16_t adcval = adc_acquire();
                float temp = adc_temp_convert(adcval);
                sk_lcd_set_addr(&lcd, 0x40);
                char tmp[20], buffer[20];
		snprintf(tmp, sk_arr_len(tmp), "%5.1f", temp);
		snprintf(buffer, sk_arr_len(buffer), "T=%-5sC A=%u", tmp, (unsigned int)adcval);
		lcd_print_text(&lcd, buffer);
        }
}
