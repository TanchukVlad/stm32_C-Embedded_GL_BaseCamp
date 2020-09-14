#include "color_sensor.h"
#include "servo.h"
#include "printf.h"
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/scb.h>

#define S1_ANGLE_1 180
#define S1_ANGLE_2 90
#define S1_ANGLE_3 5

#define POS1_DELAY 600
#define POS2_DELAY 500
#define POS3_DELAY 1000

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
        .delay_func_us = NULL,
        .delay_func_ms = &sk_tick_delay_ms,
        .is4bitinterface = 1
};

sk_pin cs_out = { .port = PORTD, .pin = 7, .isinverse = false };
sk_pin_group cs_s_group = {
        .port = PORTD,
        .pins = (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3),
        .inversions = false
};

int main(void)
{
        rcc_periph_clock_enable(RCC_GPIOA);
        rcc_periph_clock_enable(RCC_GPIOE);
        rcc_periph_clock_enable(RCC_GPIOD);

        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rs, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rw, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_en, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_bkl, MODE_OUTPUT);

        sk_pin_group_mode_setup(cs_s_group, MODE_OUTPUT);
        sk_pin_mode_setup(cs_out, MODE_INPUT);

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
        lcd_print_text(&lcd, "Candy sorting!");
        sk_tick_delay_ms(1000);
        sk_lcd_set_addr(&lcd, 0x00);
        lcd_print_text(&lcd, "Starting...   ");

        pwm_set_freq(50);   // 50Hz
        // Red, Green, Yellow, Orange, Violet, Unknown
        uint16_t count_arr[] = {0, 0, 0, 0, 0, 0};

        servo_start(SERVO_CH2);

        while (1) {
                pwm_set_servo(SERVO_CH1, S1_ANGLE_1);
                sk_tick_delay_ms(POS1_DELAY);

                pwm_set_servo(SERVO_CH1, S1_ANGLE_2);
                uint8_t rgb[3];
                color_get_rgb(cs_s_group, cs_out, 100, rgb);
                uint8_t cell = color_name(rgb);
                servo_choose_cell(SERVO_CH2, cell);
                count_arr[cell] = count_arr[cell] + 1;
                sk_tick_delay_ms(POS2_DELAY);

                pwm_set_servo(SERVO_CH1, S1_ANGLE_3);
                char buffer[32];
                snprintf(buffer, sk_arr_len(buffer), "R=%u G=%u Y=%u\nO=%u V=%u U=%u",
                        count_arr[0], count_arr[1], count_arr[2], count_arr[3],
                        count_arr[4], count_arr[5]);
                lcd_clear_display(&lcd);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_print_text(&lcd, buffer);
                sk_tick_delay_ms(POS3_DELAY);
        }
}
