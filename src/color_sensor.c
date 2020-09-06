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

sk_pin lcd_rs = { .port = PORTE, .pin = 7, .isinverse = false };
sk_pin lcd_rw = { .port = PORTE, .pin = 10, .isinverse = false };
sk_pin lcd_en = { .port = PORTE, .pin = 11, .isinverse = false };
sk_pin lcd_bkl = { .port = PORTE, .pin = 9, .isinverse = false };

sk_pin cs_out = { .port = PORTE, .pin = 6, .isinverse = false };

sk_pin_group lcd_group = {
        .port = PORTE,
        .pins = (1 << 15) | (1 << 14) | (1 << 13) | (1 << 12),
        .inversions = false
};

sk_pin_group cs_s_group = {
        .port = PORTE,
        .pins = (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2),
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
                        if(sk_pin_read(pin)) {
                                freq_cnt++;
                        } else {
                                __WFI();
                        }
                }
        } else {
                while (sk_tick_get_current() <= next) {
                        if(sk_pin_read(pin)) {
                                freq_cnt++;
                        } else {
                                __WFI();
                        }
                }
        }

        return freq_cnt / ms;
}


int main (void)
{
        rcc_periph_clock_enable(RCC_GPIOE);
        sk_pin_group_mode_setup(lcd_group, MODE_OUTPUT);
        sk_pin_group_mode_setup(cs_s_group, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rs, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_rw, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_en, MODE_OUTPUT);
        sk_pin_mode_setup(lcd_bkl, MODE_OUTPUT);

        sk_pin_mode_setup(cs_out, MODE_INPUT);

        uint32_t period = 16000000ul / 10000ul;
        uint8_t priority = 2;
        sk_tick_init(period, priority);
        cm_enable_interrupts();

        sk_pin_group_set(lcd_group, 0x00);
        lcd_poweron_delay(&lcd);
        lcd_init_4bit(&lcd);
        sk_lcd_set_backlight(&lcd, true);

        while(1)
        {
                uint32_t red_freq = 0;
                uint32_t blue_freq = 0;
                uint32_t green_freq = 0;
                //red
                sk_pin_group_set(cs_s_group, 0b0001);
                red_freq = read_freq(cs_out, 100);
                //blue
                sk_pin_group_set(cs_s_group, 0b1001);
                blue_freq = read_freq(cs_out, 100);
                //green
                sk_pin_group_set(cs_s_group, 0b1101);
                green_freq = read_freq(cs_out, 100);
                //clear
                sk_pin_group_set(cs_s_group, 0b0101);
                sk_tick_delay_ms(100);

                char red[5], blue[5], green[5], buffer[25];
		snprintf(red, sk_arr_len(red), "%d", red_freq);
                snprintf(blue, sk_arr_len(red), "%d", blue_freq);
                snprintf(green, sk_arr_len(red), "%d", green_freq);
		snprintf(buffer, sk_arr_len(buffer), "R=%s G=%s\nB=%s", red, green, blue);
                sk_lcd_set_addr(&lcd, 0x00);
                lcd_print_text(&lcd, buffer);

        }
}
