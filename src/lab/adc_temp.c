#include "lcd_hd44780.h"
#include "tick.c"
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

const sk_pin led_green = { .port=PORTD, .pin=12, .isinverse=false};
const sk_pin temp = { .port=PORTB, .pin=9, .isinverse=false};

const sk_pin lcd_rs = { .port = PORTE, .pin = 7, .isinverse = false };
const sk_pin lcd_rw = { .port = PORTE, .pin = 10, .isinverse = false };
const sk_pin lcd_en = { .port = PORTE, .pin = 11, .isinverse = false };
const sk_pin lcd_bkl = { .port = PORTE, .pin = 9, .isinverse = false };

const sk_pin_group lcd_group = {
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

static void adc_temp_init(void)
{
        rcc_periph_clock_enable(RCC_GPIOB);
        //minimize noise
        gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO9);

        sk_pin_mode_setup(temp, MODE_ANALOG);
        // APB2 prescaler /2
        rcc_set_ppre2(RCC_CFGR_PPRE_DIV_2);
        // ADC1 clock
        rcc_periph_clock_enable(RCC_ADC1);
        // ADC1 prescaler /8
        adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);

        // TODO:
}


int main (void)
{
        rcc_periph_clock_enable(RCC_GPIOD);
        rcc_periph_clock_enable(RCC_GPIOE);
        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_mode_setup(led_green, MODE_OUTPUT);
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
        lcd_init_4bit(&lcd);

        sk_lcd_set_backlight(&lcd, true);

        sk_pin_set(led_green, true);

        while(1)
        {
                sk_pin_set(led_green, true);
                sk_tick_delay_ms(1000);
                sk_pin_set(led_green, false);
                sk_tick_delay_ms(1000);
        }
}
