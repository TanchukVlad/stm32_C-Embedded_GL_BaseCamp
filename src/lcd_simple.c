#include <libopencm3/stm32/rcc.h>
#include "lcd_hd44780.h"
#include "tick.h"
#include <stdint.h>
#include <stddef.h>
#include <libopencm3/cm3/cortex.h>

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
        .is4bitinterface = true
};

int main(void)
{
    rcc_periph_clock_enable(RCC_GPIOE);
    rcc_periph_clock_enable(RCC_GPIOD);

    uint32_t period = 1000;
    uint8_t priority = 2;
    sk_tick_init(period, priority);
    cm_enable_interrupts();

    sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
    sk_pin_group_set(lcd_group, 0x00);
    lcd_init_4bit(&lcd);

    while (1) {

            sk_tick_delay_ms(500);
    }
}
