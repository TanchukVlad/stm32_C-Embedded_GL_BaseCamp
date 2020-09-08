/**
 * In this program is programed Color Sensor TCS3200.
 * Documentation about this sensor in documentation/color sensor/tcs3200-e11.pdf
 * This sensor has 5 input pins and 1 output.
 * s0, s1 - Output frequency scaling selection inputs.
 * s2, s3 - Photodiode type selection inputs.
 * OE - Enable for fo (active low).
 * OUT - Output frequency (fo).
 *
 * We set the pin group for s0, s1, s2, s3, OE.
 * OUT transmit output frequency, so this frequency is read by read_freq().
 * read_freq() was based on TIMx.
 */

#include "lcd_hd44780.h"
#include "printf.h"
#include "tick.h"
#include <libopencm3/cm3/sync.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
sk_pin green = { .port = PORTD, .pin = 12, .isinverse = false};

sk_pin lcd_rs = { .port = PORTE, .pin = 7, .isinverse = false };
sk_pin lcd_rw = { .port = PORTE, .pin = 10, .isinverse = false };
sk_pin lcd_en = { .port = PORTE, .pin = 11, .isinverse = false };
sk_pin lcd_bkl = { .port = PORTE, .pin = 9, .isinverse = false };

sk_pin cs_out = { .port = PORTD, .pin = 7, .isinverse = false };

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


uint32_t read_freq(sk_pin pin, uint32_t ms)
{
        uint32_t cur = sk_tick_get_current();
        uint32_t delta = (sk_tick_get_rate_hz() / 1000) * ms;
        uint32_t next = cur + delta;

        uint32_t freq_cnt = 0;

        if (sk_tick_get_current() > next) {
                while (sk_tick_get_current() > next) {
                        // wait cs_out = 1
                        while (!sk_pin_read(pin));
                        // wait cs_out = 0
                        while (sk_pin_read(pin));
                        //after cs_out 1 and 0 (it is one impulse) increment freq_cnt
                        freq_cnt++;
                }
        } else {
                while (sk_tick_get_current() <= next) {
                        while (!sk_pin_read(pin));

                        while (sk_pin_read(pin));

                        freq_cnt++;
                }
        }

        return freq_cnt / ms;
}


uint8_t color_scale(uint32_t freq, char color)
{
        uint8_t scale;

        switch (color) {
                case 'R':
                        scale = 25;
                        break;
                case 'G':
                        scale = 33;
                        break;
                case 'B':
                        scale = 26;
                        break;
                default:
                        scale = 25;
                        break;
        }

        return ((freq * scale) / 10 > 255) ? 255 : (freq * scale) / 10;

}


int main (void)
{
        rcc_periph_clock_enable(RCC_GPIOE);
        rcc_periph_clock_enable(RCC_GPIOD);

        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_group_mode_setup(cs_s_group, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rs, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rw, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_en, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_bkl, MODE_OUTPUT);

        sk_pin_mode_setup(green, MODE_OUTPUT);

        sk_pin_set(green, false);

        sk_pin_mode_setup(cs_out, MODE_INPUT);

        uint32_t period = 16000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        sk_pin_group_set(lcd_group, 0x00);
        lcd_poweron_delay(&lcd);
        lcd_init_4bit(&lcd);
        sk_lcd_set_backlight(&lcd, true);

        sk_pin_set(green, true);

        uint8_t red_freq = 0;
        uint8_t blue_freq = 0;
        uint8_t green_freq = 0;

        while(1)
        {
                //red
                sk_pin_group_set(cs_s_group, 0b0011);
                red_freq = color_scale(read_freq(cs_out, 10), 'R');
                //green
                sk_pin_group_set(cs_s_group, 0b1111);
                green_freq = color_scale(read_freq(cs_out, 10), 'G');
                //blue
                sk_pin_group_set(cs_s_group, 0b1011);
                blue_freq = color_scale(read_freq(cs_out, 10), 'B');
                //clear
                sk_pin_group_set(cs_s_group, 0b0111);
                sk_tick_delay_ms(10);

                char buffer[25];
                snprintf(buffer, sk_arr_len(buffer), "R=%u G=%u\nB=%u",
                        (unsigned int)red_freq, (unsigned int)green_freq, (unsigned int)blue_freq);
                lcd_clear_display(&lcd);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_print_text(&lcd, buffer);
                sk_tick_delay_ms(500);
        }
}
