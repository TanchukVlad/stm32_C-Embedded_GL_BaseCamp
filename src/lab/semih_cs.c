#include "color_sensor.h"
#include "servo.h"
#include "clock_168mhz.h"
#include "freq_read.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/timer.h>
#include <stdint.h>
#include <stddef.h>
// USE_SEMIHOSTING is defined via our Makefile
#if defined(USE_SEMIHOSTING) && USE_SEMIHOSTING
// Usually we try not to use standard C library. But in this case we need printf
#include <stdio.h>

// this is our magic from librdimon
// should be called in main before first output
extern void initialise_monitor_handles(void);
#else
// this is how semihosting may be used conditionally
#error "This example requires SEMIHOSTING=1"
#endif

sk_pin green = { .port = PORTD, .pin = 12, .isinverse = false};

sk_pin lcd_rs = { .port = PORTE, .pin = 7, .isinverse = false };
sk_pin lcd_rw = { .port = PORTE, .pin = 10, .isinverse = false };
sk_pin lcd_en = { .port = PORTE, .pin = 11, .isinverse = false };
sk_pin lcd_bkl = { .port = PORTE, .pin = 9, .isinverse = false };

sk_pin cs_out = { .port = PORTA, .pin = 5, .isinverse = false };

sk_pin_group lcd_group = {
        .port = PORTE,
        .pins = (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12),
        .inversions = false
};

sk_pin_group cs_s_group = {
        .port = PORTD,
        .pins = (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3),
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

void lcd_mode_set(void)
{
        rcc_periph_clock_enable(RCC_GPIOE);
        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rs, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rw, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_en, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_bkl, MODE_OUTPUT);
}

int main(void)
{
	initialise_monitor_handles();
	printf("Monitor initialized. WE GET IT THROUGH SEMIHOSTING\n");


        rcc_periph_clock_enable(RCC_GPIOD);

        sk_pin_group_mode_setup(cs_s_group, MODE_OUTPUT);

        sk_pin_mode_setup(green, MODE_OUTPUT);

        sk_pin_set(green, false);

        gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);

        clock_init();
        printf("rcc_apb1_frequency: %u\n", rcc_apb1_frequency);
        pwm_init();
        servo_init();
        timer3_init();
        timer2_init();

        uint32_t period = 168000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        sk_pin_group_set(lcd_group, 0x00);
        lcd_poweron_delay(&lcd);
        lcd_init_4bit(&lcd);
        sk_lcd_set_backlight(&lcd, true);

        pwm_set_freq(50);   // 50Hz
        sk_tick_delay_ms(1000);

        pwm_set_servo(SERVO_CH1, 90);

        sk_pin_set(green, true);

	printf("System initialized\n");
        while (1) {
                uint32_t rgb[3];
                //color_get_rgb(cs_s_group, cs_out, 50, rgb);

                //red
                printf("----------------------------------------------\n");
                sk_pin_group_set(cs_s_group, 0b0011);
                freq_read_start(1000);
                sk_pin_toggle(green);
                rgb[0] = freq_read();
                printf("Red: %u\n", rgb[0]);
                //green
                sk_pin_group_set(cs_s_group, 0b1111);
                freq_read_start(1000);
                sk_pin_toggle(green);
                rgb[1] = freq_read();
                printf("Green: %u\n", rgb[1]);
                //blue
                sk_pin_group_set(cs_s_group, 0b1011);
                freq_read_start(1000);
                sk_pin_toggle(green);
                rgb[2] = freq_read();
                printf("Blue: %u\n", rgb[2]);

        }
}
