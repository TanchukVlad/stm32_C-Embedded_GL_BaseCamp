#include "clock_168mhz.h"
#include "lcd_hd44780.h"
#include "delay.h"
#include "freq_read.h"
#include "color_sensor.h"
#include "servo.h"
#include "printf.h"
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <stdint.h>
#include <stddef.h>

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
        .delay_func_us = &delay_us,
        .delay_func_ms = &delay_ms,
        .is4bitinterface = 1
};


char *color_full_name(uint8_t color)
{
        char *n;
        switch (color) {
                case 0:
                        n = "Red";
                        break;
                case 1:
                        n = "Green";
                        break;
                case 2:
                        n = "Yellow";
                        break;
                case 3:
                        n = "Orange";
                        break;
                case 4:
                        n = "Violet";
                        break;
                default:
                        n = "Color?";
                        break;
        }
        return n;
}


int main (void)
{
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOE);
        rcc_periph_clock_enable(RCC_GPIOD);

        sk_pin_group_mode_setup(lcd_group, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_group_mode_setup(cs_s_group, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_rs, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_rw, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_en, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);
        sk_pin_mode_setup(lcd_bkl, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE);

        gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5);
        gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO5);
        gpio_set_af(GPIOA, GPIO_AF1, GPIO5);

        clock_init();
        servo_pwm_init();
        servo_init();
        freq_read_timer3_init();
        freq_read_timer2_init();
        freq_read_timer5_init();
        delay_timer_init();

        cm_enable_interrupts();

        sk_pin_group_set(lcd_group, 0x00);
        lcd_init_4bit(&lcd);
        sk_lcd_set_backlight(&lcd, true);

        servo_pwm_set_freq(50);   // 50Hz

        servo_pwm_set_servo(SERVO_CH1, 90);
        while(1)
        {
                uint32_t rgb[3];
                color_get_rgb(cs_s_group, 500, rgb);

                char buffer[32];
                snprintf(buffer, sk_arr_len(buffer), "R=%lu G=%lu\nB=%lu",
                        (unsigned int)rgb[0], (unsigned int)rgb[1], (unsigned int)rgb[2]);
                lcd_clear_display(&lcd);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_print_text(&lcd, buffer);
        }
}
