#include "clock_168mhz.h"
#include "lcd_hd44780.h"
#include "printf.h"
#include "delay.h"
#include "freq_read.h"

sk_pin green = { .port = PORTD, .pin = 12, .isinverse = false};

sk_pin lcd_rs = { .port = PORTE, .pin = 7, .isinverse = false };
sk_pin lcd_rw = { .port = PORTE, .pin = 10, .isinverse = false };
sk_pin lcd_en = { .port = PORTE, .pin = 11, .isinverse = false };
sk_pin lcd_bkl = { .port = PORTE, .pin = 9, .isinverse = false };

sk_pin cs_out = { .port = PORTA, .pin = 15, .isinverse = false };

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

void lcd_mode_set(void)
{
        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rs, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rw, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_en, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_bkl, MODE_OUTPUT);
}

int main(void)
{
        clock_init();

        rcc_periph_clock_enable(RCC_GPIOD);
        rcc_periph_clock_enable(RCC_GPIOE);
        rcc_periph_clock_enable(RCC_GPIOA);

        sk_pin_group_mode_setup(cs_s_group, MODE_OUTPUT);
        gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO15);
        gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO15);
        gpio_set_af(GPIOA, GPIO_AF1, GPIO15);

        sk_pin_mode_setup(green, MODE_OUTPUT);
        sk_pin_set(green, false);


        lcd_mode_set();
        timer3_init();
        timer2_init();
        timer5_init();
        delay_timer_init();
        cm_enable_interrupts();
        lcd_init_4bit(&lcd);

        sk_pin_set(green, true);

        lcd_print_text(&lcd, "Hello freq_red!");


        while (1) {
                uint32_t rgb[3];
                sk_pin_toggle(green);
                //red
                sk_pin_group_set(cs_s_group, 0b0011);
                rgb[0] = freq_read(500);
                //green
                sk_pin_group_set(cs_s_group, 0b1111);
                rgb[1] = freq_read(500);
                //blue
                sk_pin_group_set(cs_s_group, 0b1011);
                rgb[2] = freq_read(500);
                char buffer[32];
                snprintf(buffer, sk_arr_len(buffer), " R=%lu G=%lu \nB=%lu",
                         rgb[0], rgb[1], rgb[2]);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_clear_display(&lcd);
                lcd_print_text(&lcd, buffer);
        }
}
