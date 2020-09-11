#include "servo.h"
#include "lcd_hd44780.h"
#include "printf.h"


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


int main(void)
{
        rcc_periph_clock_enable(RCC_GPIOE);

        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rs, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rw, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_en, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_bkl, MODE_OUTPUT);

        clock_init();
        pwm_init();
        servo_init();

        uint32_t period = 168000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        sk_pin_group_set(lcd_group, 0x00);
        lcd_poweron_delay(&lcd);
        lcd_init_4bit(&lcd);
        sk_lcd_set_backlight(&lcd, true);

        pwm_set_freq(50);   // 50Hz

        uint16_t deg = 0;    // 0deg

        while (1) {
                pwm_set_servo(PWM_CH1, deg);
                pwm_set_servo(PWM_CH2, deg);

                char buffer[25];
                snprintf(buffer, sk_arr_len(buffer), "Duty cycle:\n %d", (unsigned int)deg);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_print_text(&lcd, buffer);
                sk_tick_delay_ms(50);

                if (deg == 180) {
                        deg = 0;
                        lcd_clear_display(&lcd);
                        sk_tick_delay_ms(2000);
                } else if (deg == 90) {
                        sk_tick_delay_ms(2000);
                } else if (deg == 0) {
                        sk_tick_delay_ms(2000);
                }
                deg += 10;
        }
}
